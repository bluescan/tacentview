# FetchContent_Sparse (TacentView project-local CMake module)
#
# A small extension of CMake's FetchContent that adds support for
# `git sparse-checkout` (partial clone). CMake's own FetchContent /
# ExternalProject has no `GIT_SPARSE_CHECKOUT` option, so this module performs
# the sparse clone itself at configure time and then registers the result with
# the real FetchContent system via FetchContent_SetPopulated(). The standard
# FetchContent_MakeAvailable(<name>) therefore works unchanged: it sees the
# dependency as already populated, does not attempt a download, and simply
# exposes <name>_SOURCE_DIR / <name>_BINARY_DIR.
#
# This module does NOT modify CMake's own FetchContent.cmake. Pull it in with:
#
#     list(APPEND CMAKE_MODULE_PATH "<source>/CMakeModules")
#     include(FetchContent_Sparse)
#
# Usage:
#
#     FetchContent_Sparse_Declare(
#         myfixtures
#         GIT_REPOSITORY https://github.com/someone/repo.git
#         GIT_TAG        master            # a branch, a tag, OR a commit SHA
#         GIT_SPARSE_CHECKOUT Images       # where to grab from: the 'Images' tree
#         [SOURCE_DIR <dir>]               # where the files land; default ${FETCHCONTENT_BASE_DIR}/myfixtures-src
#         [QUIET]
#     )
#     FetchContent_MakeAvailable(myfixtures)
#
# GIT_SPARSE_CHECKOUT names the path(s) in the repository to fetch (e.g. 'Images'),
# i.e. where to grab the files FROM. SOURCE_DIR is the final destination: the
# CONTENTS of the first listed path are copied directly into it, so
#     GIT_SPARSE_CHECKOUT Images   +   SOURCE_DIR <dir>   ->   <dir>/<each item in 'Images'>
# If several paths are listed, their contents are all merged into SOURCE_DIR.
# The temporary clone (and its .git) is removed afterwards, so only the requested
# content is left behind. SOURCE_DIR must be a subdirectory (or a build-tree
# path), never the source root itself.
#
# Revision (GIT_TAG) and auto-tracking:
#   * branch  (e.g. master)  -> tracked: `git ls-remote` reports the current tip;
#                               if it moved since the last fetch, the content is
#                               re-fetched. (Mirrors FetchContent's "online" update
#                               for a moving ref.)
#   * tag       (e.g. v1.2)  -> tracked the same way (re-fetch if the tag moved).
#   * SHA       (40-hex)     -> pinned/immutable: never tracked, never re-fetched,
#                               and needs no network once populated. The most
#                               deterministic and offline-robust option.
#
# Offline / disconnected (mirrors FetchContent's own cache vars, global or per-name
# e.g. FETCHCONTENT_FULLY_DISCONNECTED_MYFIXTURES):
#   * FETCHCONTENT_UPDATES_DISCONNECTED  -> skip the "has it moved?" check; use
#                                          cached content if present (else fetch once).
#   * FETCHCONTENT_FULLY_DISCONNECTED   -> no network at all; cached content is
#                                          required (fatal if absent).
#
# Manual refresh (force a re-fetch even if the ref looks unchanged):
#   -DFETCHCONTENT_SPARSE_REFRESH=ON                  (every sparse dep)
#   -DFETCHCONTENT_SPARSE_REFRESH=myfixtures           (just that one)
#
# Deletions propagate: every re-fetch MIRRORS the destination (clears it, then
# copies), so files removed upstream are removed from SOURCE_DIR too.

include_guard(GLOBAL)

include(FetchContent)   # Idempotent; provides FetchContent_SetPopulated().


# Run a git command (passed as a ;-list in `commandList`) and fail the configure
# step with useful diagnostics if it returns a non-zero exit code.
function(FetchContent_Sparse_RunGit step commandList)
    execute_process(
        COMMAND ${commandList}
        RESULT_VARIABLE   result
        OUTPUT_VARIABLE   out
        ERROR_VARIABLE    err
    )

    if(NOT result EQUAL 0)
        string(REPLACE ";" " " commandText "${commandList}")
        message(STATUS "FetchContent_Sparse: git step '${step}' failed (code ${result}).")
        message(STATUS "  command: ${commandText}")
        if(out)
            string(STRIP "${out}" out)
            message(STATUS "  stdout: ${out}")
        endif()
        if(err)
            string(STRIP "${err}" err)
            message(STATUS "  stderr: ${err}")
        endif()
        message(FATAL_ERROR "FetchContent_Sparse: git step '${step}' failed (exit code ${result}).")
    endif()
endfunction()

# --- Safety: never allow a deletion to hit a source tree --------------------
# True when `child` is `base` itself or lives anywhere inside `base`
# (path-component aware, so "foo" does not match "foobar").
function(FetchContent_Sparse_IsInside child base outVar)
    get_filename_component(child "${child}" ABSOLUTE)
    get_filename_component(base  "${base}"  ABSOLUTE)
    if(child STREQUAL base)
        set(${outVar} TRUE PARENT_SCOPE)
        return()
    endif()
    string(LENGTH    "${base}"  baseLen)
    string(SUBSTRING "${child}" 0 ${baseLen} childPrefix)
    if(childPrefix STREQUAL base)
        string(LENGTH    "${child}" childLen)
        if(NOT childLen EQUAL baseLen)
            string(SUBSTRING "${child}" ${baseLen} 1 nextChar)
            if(nextChar STREQUAL "/" OR nextChar STREQUAL "\\")
                set(${outVar} TRUE PARENT_SCOPE)
                return()
            endif()
        endif()
    endif()
    set(${outVar} FALSE PARENT_SCOPE)
endfunction()

# Delete `dir` only if it is not a source tree and not an ancestor of one.
# We only ever intend to remove our own scratch clone (inside the build tree),
# so this is a hard guard against ever wiping the project by mistake.
function(FetchContent_Sparse_SafeRemove dir)
    get_filename_component(dir "${dir}" ABSOLUTE)
    foreach(protected IN ITEMS ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
        if(protected STREQUAL "")
            continue()
        endif()
        get_filename_component(protected "${protected}" ABSOLUTE)
        if(dir STREQUAL protected)
            message(FATAL_ERROR
                "FetchContent_Sparse: refusing to remove '${dir}' (it is a source directory).")
        endif()
        FetchContent_Sparse_IsInside("${protected}" "${dir}" _inside)
        if(_inside)
            message(FATAL_ERROR
                "FetchContent_Sparse: refusing to remove '${dir}' (it contains source directory '${protected}').")
        endif()
    endforeach()
    file(REMOVE_RECURSE "${dir}")
endfunction()

# Sparse-checkout capable FetchContent_Declare. See the header for usage.
# --- Auto-track helpers -------------------------------------------------------

# Resolve a FetchContent-style boolean option, honouring both the global and the
# per-name form (e.g. FETCHCONTENT_UPDATES_DISCONNECTED and
# FETCHCONTENT_UPDATES_DISCONNECTED_<NAME>). Emits the value, or OFF.
function(FetchContent_Sparse_GetOpt name opt outVar)
    string(TOUPPER "${name}" _ucName)
    if(DEFINED "FETCHCONTENT_${opt}_${_ucName}")
        set(${outVar} "FETCHCONTENT_${opt}_${_ucName}" PARENT_SCOPE)
    elseif(DEFINED "FETCHCONTENT_${opt}")
        set(${outVar} "FETCHCONTENT_${opt}" PARENT_SCOPE)
    else()
        set(${outVar} "OFF" PARENT_SCOPE)
    endif()
endfunction()

# True when `rev` looks like a commit hash (7-64 hex chars: SHA-1 is 40,
# SHA-256 is 64). Such a revision is treated as fixed (it cannot move), so it
# is never tracked or re-polled. CMake's regex engine does not support the
# {n,m} interval quantifier, so the length is checked with string(LENGTH).
function(FetchContent_Sparse_IsHash rev outVar)
    set(${outVar} FALSE PARENT_SCOPE)
    if("${rev}" MATCHES "^[0-9a-fA-F]+$")
        string(LENGTH "${rev}" _len)
        if(_len GREATER_EQUAL 7 AND _len LESS_EQUAL 64)
            set(${outVar} TRUE PARENT_SCOPE)
        endif()
    endif()
endfunction()

# Resolve the current tip commit of `ref` in `repo` via `git ls-remote`. Emits
# the 40-hex SHA, or an empty string if the remote is unreachable (offline) or
# the ref is not found. This is the module's only "am I up to date?" network call.
function(FetchContent_Sparse_ResolveTip gitExe repo ref outVar)
    execute_process(
        COMMAND "${gitExe}" ls-remote "${repo}" "${ref}"
        RESULT_VARIABLE _lr
        OUTPUT_VARIABLE _lo
        ERROR_VARIABLE _le
    )
    set(_match "")
    if(_lr EQUAL 0 AND _lo MATCHES "^([0-9a-fA-F]+)")
        set(_match "${CMAKE_MATCH_1}")
    endif()
    string(LENGTH "${_match}" _len)
    if(_len GREATER_EQUAL 7)
        set(${outVar} "${_match}" PARENT_SCOPE)
    else()
        set(${outVar} "" PARENT_SCOPE)
    endif()
endfunction()

function(FetchContent_Sparse_Declare contentName)
    cmake_parse_arguments(FC
        "QUIET"                                # flags
        "GIT_REPOSITORY;GIT_TAG;SOURCE_DIR"    # single-value keywords
        "GIT_SPARSE_CHECKOUT"                  # multi-value keywords
        ${ARGN}
    )

    if(NOT FC_GIT_REPOSITORY)
        message(FATAL_ERROR "FetchContent_Sparse_Declare(${contentName}): GIT_REPOSITORY is required.")
    endif()
    if(NOT FC_GIT_SPARSE_CHECKOUT)
        message(FATAL_ERROR "FetchContent_Sparse_Declare(${contentName}): GIT_SPARSE_CHECKOUT <path>... is required.")
    endif()

    find_package(Git REQUIRED)
    if(NOT GIT_EXECUTABLE)
        find_program(GIT_EXECUTABLE NAMES git)
    endif()
    if(NOT GIT_EXECUTABLE)
        # Last resort: rely on 'git' being on PATH (execute_process resolves it).
        set(GIT_EXECUTABLE "git")
    endif()

    # Where the kept content will live. Kept paths are reproduced as
    # <SOURCE_DIR>/<sparse-path>.
    if(NOT FC_SOURCE_DIR)
        if(NOT DEFINED FETCHCONTENT_BASE_DIR)
            set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/_deps")
        endif()
        set(FC_SOURCE_DIR "${FETCHCONTENT_BASE_DIR}/${contentName}-src")
    endif()

    # Scratch locations (kept out of the source tree).
    set(cloneDir  "${CMAKE_BINARY_DIR}/_deps/${contentName}-clone")
    set(binaryDir "${CMAKE_BINARY_DIR}/_deps/${contentName}-build")

    # Where the fetch bookkeeping (stamp) lives. Kept in the build tree, never
    # in the source tree.
    set(stampFile "${binaryDir}/.FetchContent_Sparse_${contentName}.stamp")
    list(JOIN "${FC_GIT_SPARSE_CHECKOUT}" "," pathsJoined)

    # Is the requested revision a fixed commit hash? Branches/tags can move and
    # are auto-tracked; a hash cannot move and is never re-polled.
    FetchContent_Sparse_IsHash("${FC_GIT_TAG}" _isHash)

    file(GLOB _presentContent "${FC_SOURCE_DIR}/*")
    if(_presentContent)
        set(_contentPresent ON)
    else()
        set(_contentPresent OFF)
    endif()

    # --- Offline / disconnected switches (mirror FetchContent's own names) ----
    #   FETCHCONTENT_UPDATES_DISCONNECTED[_<NAME>] : skip the "has it moved?"
    #       check; use cached content if present, otherwise fetch once.
    #   FETCHCONTENT_FULLY_DISCONNECTED[_<NAME>]   : no network at all; the
    #       cached content must already exist.
    FetchContent_Sparse_GetOpt(${contentName} "UPDATES_DISCONNECTED" _updatesDisc)
    FetchContent_Sparse_GetOpt(${contentName} "FULLY_DISCONNECTED"   _fullyDisc)
    if(NOT _updatesDisc MATCHES "^(ON|TRUE|YES|1)$")
        set(_updatesDisc OFF)
    endif()
    if(NOT _fullyDisc MATCHES "^(ON|TRUE|YES|1)$")
        set(_fullyDisc OFF)
    endif()

    # --- Manual refresh hatch (force a re-fetch even if the ref looks current) -
    #   -DFETCHCONTENT_SPARSE_REFRESH=ON                 -> every sparse dep
    #   -DFETCHCONTENT_SPARSE_REFRESH=<name>[;<name>...]  -> just those
    set(_refresh OFF)
    if(DEFINED FETCHCONTENT_SPARSE_REFRESH AND NOT FETCHCONTENT_SPARSE_REFRESH STREQUAL "OFF")
        if(FETCHCONTENT_SPARSE_REFRESH MATCHES "^(ON|TRUE|YES|1)$"
           OR "${contentName}" IN_LIST FETCHCONTENT_SPARSE_REFRESH)
            set(_refresh ON)
        endif()
    endif()

    # ---- Disconnected fast-paths -------------------------------------------
    if(_fullyDisc OR _updatesDisc)
        if(_contentPresent)
            message(STATUS "FetchContent_Sparse(${contentName}): disconnected; using cached content at ${FC_SOURCE_DIR}.")
            FetchContent_SetPopulated(${contentName} SOURCE_DIR "${FC_SOURCE_DIR}" BINARY_DIR "${binaryDir}")
            return()
        endif()
        if(_fullyDisc)
            message(FATAL_ERROR "FetchContent_Sparse(${contentName}): FETCHCONTENT_FULLY_DISCONNECTED is set but there is no cached content at ${FC_SOURCE_DIR}.")
        endif()
        # _updatesDisc with no cached content: download once (no "update" check).
        set(_track OFF)
    else()
        set(_track ON)
    endif()

    # ---- Resolve the current tip (online, unless tracking is disabled) ------
    # For a fixed hash this is the hash itself (no network). For a branch/tag it
    # is `git ls-remote` -- the only network call used to decide whether to
    # re-fetch. An unreachable remote (offline) degrades gracefully: use the
    # cached content if we have some, otherwise fail with a clear message.
    set(_curTip "")
    if(_track)
        if(_isHash)
            string(TOLOWER "${FC_GIT_TAG}" _curTip)
        else()
            FetchContent_Sparse_ResolveTip("${GIT_EXECUTABLE}" "${FC_GIT_REPOSITORY}" "${FC_GIT_TAG}" _curTip)
            if(NOT _curTip)
                if(_contentPresent)
                    message(STATUS "FetchContent_Sparse(${contentName}): cannot reach ${FC_GIT_REPOSITORY} (offline); using cached content at ${FC_SOURCE_DIR}.")
                    FetchContent_SetPopulated(${contentName} SOURCE_DIR "${FC_SOURCE_DIR}" BINARY_DIR "${binaryDir}")
                    return()
                endif()
                message(FATAL_ERROR "FetchContent_Sparse(${contentName}): cannot reach ${FC_GIT_REPOSITORY} to resolve '${FC_GIT_TAG}', and there is no cached content at ${FC_SOURCE_DIR}.")
            endif()
        endif()
    endif()

    # ---- Up-to-date check (skip the fetch) ----------------------------------
    # The stamp records the exact commit we materialised last time. If the
    # current tip is that same commit, the repo/paths are unchanged, the content
    # is present, and we are not forcing a refresh, there is nothing to do.
    if(NOT _refresh)
        set(_upToDate OFF)
        if(EXISTS "${stampFile}")
            file(READ "${stampFile}" _oldStamp)
            string(REPLACE "|" ";" _oldParts "${_oldStamp}")
            list(LENGTH _oldParts _oldN)
            if(_oldN EQUAL 4)
                list(GET _oldParts 0 _oldRepo)
                list(GET _oldParts 1 _oldRef)
                list(GET _oldParts 2 _oldSha)
                list(GET _oldParts 3 _oldPaths)
                if(_oldRepo STREQUAL "${FC_GIT_REPOSITORY}"
                   AND _oldRef  STREQUAL "${FC_GIT_TAG}"
                   AND _oldPaths STREQUAL "${pathsJoined}"
                   AND _contentPresent)
                    if(_isHash)
                        # User's hash (full or short) must prefix the stored full commit.
                        if(_oldSha MATCHES "^${FC_GIT_TAG}")
                            set(_upToDate ON)
                        endif()
                    elseif(_oldSha STREQUAL "${_curTip}")
                        set(_upToDate ON)
                    endif()
                endif()
            endif()
        endif()
        if(_upToDate)
            message(STATUS "FetchContent_Sparse(${contentName}): up to date at ${_curTip}; skipping fetch.")
            FetchContent_SetPopulated(${contentName} SOURCE_DIR "${FC_SOURCE_DIR}" BINARY_DIR "${binaryDir}")
            return()
        endif()
    endif()

    # ---- Fetch (online) ------------------------------------------------------
    FetchContent_Sparse_SafeRemove("${cloneDir}")
    message(STATUS "FetchContent_Sparse(${contentName}): fetching '${FC_GIT_TAG}' from ${FC_GIT_REPOSITORY}; sparse paths: ${FC_GIT_SPARSE_CHECKOUT}")

    # 1) Partial clone without a working tree (all refs, no blobs). We do NOT use
    #    `--branch` (it rejects commit hashes); the revision is checked out below,
    #    which works for a branch, a tag, or a SHA.
    set(cloneArgs "${GIT_EXECUTABLE}" clone --filter=blob:none --no-checkout "${FC_GIT_REPOSITORY}" "${cloneDir}")
    FetchContent_Sparse_RunGit("clone" "${cloneArgs}")

    # 2) Enable cone-mode sparse checkout and select the paths to keep.
    set(sparseArgs "${GIT_EXECUTABLE}" -C "${cloneDir}" sparse-checkout set --cone)
    foreach(path IN LISTS FC_GIT_SPARSE_CHECKOUT)
        list(APPEND sparseArgs "${path}")
    endforeach()
    FetchContent_Sparse_RunGit("sparse-checkout set" "${sparseArgs}")

    # 3) Make the requested revision available (a no-op if already present),
    #    then check it out and materialise the sparse working tree.
    set(fetchArgs "${GIT_EXECUTABLE}" -C "${cloneDir}" fetch --filter=blob:none origin "${FC_GIT_TAG}")
    execute_process(COMMAND ${fetchArgs} RESULT_VARIABLE _fetchRev OUTPUT_VARIABLE _fetchOut ERROR_VARIABLE _fetchErr)
    set(checkoutArgs "${GIT_EXECUTABLE}" -C "${cloneDir}" checkout -f "${FC_GIT_TAG}")
    FetchContent_Sparse_RunGit("checkout" "${checkoutArgs}")
    set(resetArgs "${GIT_EXECUTABLE}" -C "${cloneDir}" reset --hard)
    FetchContent_Sparse_RunGit("materialize" "${resetArgs}")

    # 4) Record the exact commit we are materialising (for the stamp + tracking).
    set(headArgs "${GIT_EXECUTABLE}" -C "${cloneDir}" rev-parse HEAD)
    execute_process(COMMAND ${headArgs} RESULT_VARIABLE _headRes OUTPUT_VARIABLE _headSha ERROR_VARIABLE _headErr)
    if(NOT _headRes EQUAL 0 OR NOT _headSha)
        message(FATAL_ERROR "FetchContent_Sparse(${contentName}): could not determine the checked-out commit.")
    endif()
    string(STRIP "${_headSha}" _headSha)

    # 5) Verify every sparse path exists and is non-empty in the scratch clone
    #    BEFORE touching the destination, so a bad fetch can't wipe good content.
    set(_copyItems "")
    foreach(path IN LISTS FC_GIT_SPARSE_CHECKOUT)
        set(srcPath "${cloneDir}/${path}")
        if(NOT IS_DIRECTORY "${srcPath}")
            message(FATAL_ERROR "FetchContent_Sparse(${contentName}): sparse path '${path}' not found in ${FC_GIT_REPOSITORY} (rev: ${FC_GIT_TAG}).")
        endif()
        file(GLOB _items  "${srcPath}/*")
        file(GLOB _hidden "${srcPath}/.[^ ]*")   # also pick up dotfiles
        list(APPEND _items ${_hidden})
        if(NOT _items)
            message(FATAL_ERROR "FetchContent_Sparse(${contentName}): sparse path '${path}' is empty.")
        endif()
        list(APPEND _copyItems ${_items})
    endforeach()

    # 6) MIRROR the destination: clear it (guarded), then copy the fresh content.
    #    Clearing first is what makes *deletions* propagate; SafeRemove refuses
    #    to touch the source tree or anything above it, so this is safe.
    FetchContent_Sparse_SafeRemove("${FC_SOURCE_DIR}")
    file(MAKE_DIRECTORY "${FC_SOURCE_DIR}")
    file(COPY ${_copyItems} DESTINATION "${FC_SOURCE_DIR}")

    # 7) Drop the temporary clone (including its .git); keep only the content.
    FetchContent_Sparse_SafeRemove("${cloneDir}")

    # 8) Record what we fetched (the exact commit) so future configures can skip.
    file(MAKE_DIRECTORY "${binaryDir}")
    file(WRITE "${stampFile}" "${FC_GIT_REPOSITORY}|${FC_GIT_TAG}|${_headSha}|${pathsJoined}")

    # 9) Register with the real FetchContent so FetchContent_MakeAvailable()
    #    reports the dependency as already populated and does not re-download.
    FetchContent_SetPopulated(${contentName}
        SOURCE_DIR  "${FC_SOURCE_DIR}"
        BINARY_DIR  "${binaryDir}")

    message(STATUS "FetchContent_Sparse(${contentName}): populated ${_headSha} -> ${FC_SOURCE_DIR}")
endfunction()