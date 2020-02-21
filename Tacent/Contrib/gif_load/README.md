# gif_load
This is an ANSI C compatible animated GIF loader in a single header file of
less than 300 lines of code (less than 200 without empty lines and comments).
It defines 1 new struct and 1 new enum, and requires 1 function call to load
a GIF. 'ANSI C compatible' means that it builds fine with `-pedantic -ansi`
compiler flags but includes `stdint.h` which is unavailable prior to C99.

`gif_load` is free and unencumbered software released into the public domain,
blah blah. See the header file for details.

There are no strict dependencies on the standard C library. The only external
function used by default is `realloc()` (both for freeing and allocation), but
it\`s possible to override it by defining a macro called `GIF_MGET(m,s,a,c)`
prior to including the header; `m` stands for a `uint8_t*`-typed pointer to
the memory block being allocated or freed, `s` is the target block size, typed
`unsigned long`, `a` is the value of the fifth parameter passed to `GIF_Load()`
(mainly used to hold a pointer to some user-defined structure if need be; see
below), typed `void*`, and `c` equals 0 on freeing and 1 on allocation. For
example, `GIF_MGET` might be defined as follows if `malloc()` / `free()` pair
is to be used instead of `realloc()`:

```c
#include <stdlib.h>
#define GIF_MGET(m,s,a,c) if (c) m = (uint8_t*)malloc(s); else free(m);
#include "gif_load.h"
```

Loading GIFs immediately from disk is not supported: target files must
be read or otherwise mapped into RAM by the caller.

The main function that does the actual loading is called `GIF_Load()`.
It requires a callback function to create the animation structure of
any user-defined format. This function, further referred to as the
'frame writer callback', will be executed once every frame.

Aditionally, the main function accepts a second callback used to process GIF
[application-specific extensions](https://stackoverflow.com/a/28486261/7019311),
i.e. metadata; in the vast majority of GIFs no such extensions are present, so
this callback is optional.

Both frame writer callback and metadata callback need 2 parameters:

1. callback-specific data
2. pointer to a `struct GIF_WHDR` that encapsulates GIF frame information
(callbacks may alter any fields at will, as the structure passed to them is a
proxy that is discarded after every call):
  * `GIF_WHDR::xdim` - global GIF width, always constant across frames
                       (further referred to as 'ACAF'); [0; 65535]
  * `GIF_WHDR::ydim` - global GIF height, ACAF; [0; 65535]
  * `GIF_WHDR::clrs` - number of colors in the current palette (local palettes
                       are not that rare so it may vary across frames, further
                       referred to as 'MVAF'); {2; 4; 8; 16; 32; 64; 128; 256}
  * `GIF_WHDR::bkgd` - 0-based background color index for the current palette,
                       ACAF (sic ACAF, as this index is set globally)
  * `GIF_WHDR::tran` - 0-based transparent color index for the current palette
                       (or −1 when transparency is disabled), MVAF
  * `GIF_WHDR::intr` - boolean flag indicating whether the current frame is
                  [interlaced](https://en.wikipedia.org/wiki/GIF#Interlacing);
                       deinterlacing it is up to the caller (see the examples
                       below), MVAF
  * `GIF_WHDR::mode` - next frame (sic next, not current) blending mode, MVAF:
                       [`GIF_NONE`:] no blending, mainly used in single-frame
                       GIFs, functionally equivalent to `GIF_CURR`;
                       [`GIF_CURR`:] leave the current image state as is;
                       [`GIF_BKGD`:] restore the background color (or
                       transparency, in case `GIF_WHDR::tran` ≠ −1) in the
                       boundaries of the current frame; [`GIF_PREV`:] restore
                       the last image whose mode differed from this one,
                       functionally equivalent to `GIF_BKGD` when assigned to
                       the first frame in a GIF; *N.B.:* if right before a
                       `GIF_PREV` frame came a `GIF_BKGD` one, the state to
                       be restored is before a certain part of the resulting
                       image was filled with the background color, not after!
  * `GIF_WHDR::frxd` - current frame width, MVAF; [0; 65535]
  * `GIF_WHDR::fryd` - current frame height, MVAF; [0; 65535]
  * `GIF_WHDR::frxo` - current frame horizontal offset, MVAF; [0; 65535]
  * `GIF_WHDR::fryo` - current frame vertical offset, MVAF; [0; 65535]
  * `GIF_WHDR::time` - next frame delay in GIF time units (1 unit = 10 msec),
                       MVAF; negative values are possible here, they mean
                       that the frame requires user input to advance, and the
                       actual delay equals −(`time` + 1) GIF time units: zero
                       delay + user input = wait for input indefinitely,
                       nonzero delay + user input = wait for either input or
                       timeout (whichever comes first); *N.B.:* user input
                       requests can be safely ignored, disregarding the GIF
                       standard
  * `GIF_WHDR::ifrm` - 0-based index of the current frame, always varies
                       across frames (further referred to as 'AVAF')
  * `GIF_WHDR::nfrm` - total frame count, negative if the GIF data supplied
                       is incomplete, ACAF during a single `GIF_Load()` call
                       but may vary across `GIF_Load()` calls
  * `GIF_WHDR::bptr` - [frame writer:] pixel indices for the current frame,
                       ACAF (it is only the pointer address that is constant;
                       the pixel indices stored inside = MVAF); [metadata
                       callback:] app metadata header (8+3 bytes) followed by
                       a GIF chunk (1 byte designating length L, then L bytes
                       of metadata, and so forth; L = 0 means end of chunk),
                       AVAF
  * `GIF_WHDR::cpal` - the current palette containing 3 `uint8_t` values for
                       each of the colors: `R` for the red channel, `G` for
                       green and `B` for blue; this pointer is guaranteed
                       to be the same across frames if and only if the global
                       palette is used for those frames (local palettes are
                       strictly frame-specific, even when they contain the
                       same number of identical colors in identical order),
                       MVAF

Neither of the two callbacks needs to return a value, thus having `void` for
a return type.

`GIF_Load()`, in its turn, needs 6 parameters:

1. a pointer to GIF data in RAM
2. GIF data size; may be larger than the actual data if the GIF has a proper
   ending mark
3. a pointer to the frame writer callback
4. a pointer to the metadata callback; may be left empty
5. callback-specific data
6. number of frames to skip before executing the callback; useful to resume
   loading the partial file

Partial GIFs are also supported, but only at a granularity of one frame. For
example, if the file ends in the middle of the fifth frame, no attempt would
be made to recover the upper half, and the resulting animation will only
contain 4 frames. When more data is available the loader might be called
again, this time with the `skip` parameter equalling 4 to skip those 4 frames.
Note that the metadata callback is not affected by `skip` and gets called
again every time the frames between which the metadata was written are
skipped.

The return value of the function above, if positive, equals the total number
of frames in the animation and indicates that the GIF data stream ended with
a proper termination mark. Negative return value is the number of frames
loaded per current call multiplied by −1, suggesting that the GIF data stream
being decoded is still incomplete. Zero, in its turn, means that the call
could not decode any more frames.

`gif_load` is endian-aware. If the target machine can be big-endian the user
has to determine that manually and add `#define GIF_BIGE 1` to the source
prior to the header being included if that\`s the case, or otherwise define
the endianness to be used (0 = little, 1 = big), e.g. by declaring a helper
function and setting `GIF_BIGE` to expand into its call, or by passing it as a
compiler parameter (e.g. `-DGIF_BIGE=1` for GCC / Clang). Although GIF data is
little-endian, all multibyte integers passed to the user through `long`-typed
fields of `GIF_WHDR` have correct byte order regardless of the endianness of
the target machine, provided that `GIF_BIGE` is set correctly. Most other
data, e.g. pixel indices of a frame, consists of single bytes and thus does
not require endianness correction. One notable exception is GIF application
metadata which is passed as the raw chunk of bytes (for details see the
description of `GIF_WHDR::bptr` provided above), and then it\`s the
callback\`s job to parse it and decide whether to decode and how to do that.

There is a possibility to build `gif_load` as a shared library. `GIF_EXTR` is
a convenience macro to be defined so that the `GIF_Load()` function gets an
entry in the export table of the library. See the Python example for further
information.



# C / C++ usage example
Here is an example of how to use `GIF_Load()` to transform an animated GIF
file into a 32-bit uncompressed TGA. For the sake of simplicity all frames
are concatenated one below the other and no attempt is made to keep all of
them if the resulting height exceeds the TGA height limit which is 0xFFFF.

```c
#include "gif_load.h"
#include <fcntl.h>
#ifdef _MSC_VER
    /** MSVC is definitely not my favourite compiler...   >_<   **/
    #pragma warning(disable:4996)
    #include <io.h>
#else
    #include <unistd.h>
#endif
#ifndef O_BINARY
    #define O_BINARY 0
#endif

#pragma pack(push, 1)
typedef struct {
    void *data, *pict, *prev;
    unsigned long size, last;
    int uuid;
} STAT; /** #pragma avoids -Wpadded on 64-bit machines **/
#pragma pack(pop)

void Frame(void*, struct GIF_WHDR*); /** keeps -Wmissing-prototypes happy **/
void Frame(void *data, struct GIF_WHDR *whdr) {
    uint32_t *pict, *prev, x, y, yoff, iter, ifin, dsrc, ddst;
    uint8_t head[18] = {0};
    STAT *stat = (STAT*)data;

    #define BGRA(i) ((whdr->bptr[i] == whdr->tran)? 0 : \
          ((uint32_t)(whdr->cpal[whdr->bptr[i]].R << ((GIF_BIGE)? 8 : 16)) \
         | (uint32_t)(whdr->cpal[whdr->bptr[i]].G << ((GIF_BIGE)? 16 : 8)) \
         | (uint32_t)(whdr->cpal[whdr->bptr[i]].B << ((GIF_BIGE)? 24 : 0)) \
         | ((GIF_BIGE)? 0xFF : 0xFF000000)))
    if (!whdr->ifrm) {
        /** TGA doesn`t support heights over 0xFFFF, so we have to trim: **/
        whdr->nfrm = ((whdr->nfrm < 0)? -whdr->nfrm : whdr->nfrm) * whdr->ydim;
        whdr->nfrm = (whdr->nfrm < 0xFFFF)? whdr->nfrm : 0xFFFF;
        /** this is the very first frame, so we must write the header **/
        head[ 2] = 2;
        head[12] = (uint8_t)(whdr->xdim     );
        head[13] = (uint8_t)(whdr->xdim >> 8);
        head[14] = (uint8_t)(whdr->nfrm     );
        head[15] = (uint8_t)(whdr->nfrm >> 8);
        head[16] = 32;   /** 32 bits depth **/
        head[17] = 0x20; /** top-down flag **/
        write(stat->uuid, head, 18UL);
        ddst = (uint32_t)(whdr->xdim * whdr->ydim);
        stat->pict = calloc(sizeof(uint32_t), ddst);
        stat->prev = calloc(sizeof(uint32_t), ddst);
    }
    /** [TODO:] the frame is assumed to be inside global bounds,
                however it might exceed them in some GIFs; fix me. **/
    pict = (uint32_t*)stat->pict;
    ddst = (uint32_t)(whdr->xdim * whdr->fryo + whdr->frxo);
    ifin = (!(iter = (whdr->intr)? 0 : 4))? 4 : 5; /** interlacing support **/
    for (dsrc = (uint32_t)-1; iter < ifin; iter++)
        for (yoff = 16U >> ((iter > 1)? iter : 1), y = (8 >> iter) & 7;
             y < (uint32_t)whdr->fryd; y += yoff)
            for (x = 0; x < (uint32_t)whdr->frxd; x++)
                if (whdr->tran != (long)whdr->bptr[++dsrc])
                    pict[(uint32_t)whdr->xdim * y + x + ddst] = BGRA(dsrc);
    write(stat->uuid, pict, sizeof(uint32_t) * (uint32_t)whdr->xdim
                                             * (uint32_t)whdr->ydim);
    if ((whdr->mode == GIF_PREV) && !stat->last) {
        whdr->frxd = whdr->xdim;
        whdr->fryd = whdr->ydim;
        whdr->mode = GIF_BKGD;
        ddst = 0;
    }
    else {
        stat->last = (whdr->mode == GIF_PREV)?
                      stat->last : (unsigned long)(whdr->ifrm + 1);
        pict = (uint32_t*)((whdr->mode == GIF_PREV)? stat->pict : stat->prev);
        prev = (uint32_t*)((whdr->mode == GIF_PREV)? stat->prev : stat->pict);
        for (x = (uint32_t)(whdr->xdim * whdr->ydim); --x;
             pict[x - 1] = prev[x - 1]);
    }
    if (whdr->mode == GIF_BKGD) /** cutting a hole for the next frame **/
        for (whdr->bptr[0] = (uint8_t)((whdr->tran >= 0)?
                                        whdr->tran : whdr->bkgd), y = 0,
             pict = (uint32_t*)stat->pict; y < (uint32_t)whdr->fryd; y++)
            for (x = 0; x < (uint32_t)whdr->frxd; x++)
                pict[(uint32_t)whdr->xdim * y + x + ddst] = BGRA(0);
    #undef BGRA
}

int main(int argc, char *argv[]) {
    STAT stat = {0};

    if (argc < 3)
        write(1, "arguments: <in>.gif <out>.tga (1 or more times)\n", 48UL);
    for (stat.uuid = 2, argc -= (~argc & 1); argc >= 3; argc -= 2) {
        if ((stat.uuid = open(argv[argc - 2], O_RDONLY | O_BINARY)) <= 0)
            return 1;
        stat.size = (unsigned long)lseek(stat.uuid, 0UL, 2 /** SEEK_END **/);
        lseek(stat.uuid, 0UL, 0 /** SEEK_SET **/);
        read(stat.uuid, stat.data = realloc(0, stat.size), stat.size);
        close(stat.uuid);
        unlink(argv[argc - 1]);
        stat.uuid = open(argv[argc - 1], O_CREAT | O_WRONLY | O_BINARY, 0644);
        if (stat.uuid > 0) {
            GIF_Load(stat.data, (long)stat.size, Frame, 0, (void*)&stat, 0L);
            stat.pict = realloc(stat.pict, 0L);
            stat.prev = realloc(stat.prev, 0L);
            close(stat.uuid);
            stat.uuid = 0;
        }
        stat.data = realloc(stat.data, 0L);
    }
    return stat.uuid;
}
```



# Python usage example
Here is an example of how to use `GIF_Load()` from Python 2.x or 3.x.

*N.B.:* The implementation shown here complies to the GIF standard much
better than the one PIL has (at least as of 2018-02-07): for example
it preserves transparency and supports local frame palettes.

First of all, `gif_load.h` has to be built as a shared library:

Linux / macOS:
```bash
# Only works when executed from the directory where gif_load.h resides
rm gif_load.so 2>/dev/null
uname -s | grep -q ^Darwin && CC=clang || CC=gcc
$CC -pedantic -ansi -s -DGIF_EXTR=extern -xc gif_load.h -o gif_load.so \
    -shared -fPIC -Wl,--version-script=<(echo "{global:GIF_Load;local:*;};")
```

Windows:
```bash
rem Only works when executed from the directory where gif_load.h resides
del gif_load.exp gif_load.lib gif_load.dll
cl /LD /Zl /DGIF_EXTR=__declspec(dllexport) /Tc gif_load.h msvcrt.lib /Fegif_load.dll
```

Then the loading function can be called using CTypes
([Python 2 reference](https://docs.python.org/2/library/ctypes.html),
 [Python 3 reference](https://docs.python.org/3/library/ctypes.html)):

```python
from PIL import Image

def GIF_Load(file):
    from platform import system
    from ctypes import string_at, Structure, c_long as cl, c_ubyte, \
                       py_object, pointer, POINTER as PT, CFUNCTYPE, CDLL
    class GIF_WHDR(Structure): _fields_ = \
       [("xdim", cl), ("ydim", cl), ("clrs", cl), ("bkgd", cl),
        ("tran", cl), ("intr", cl), ("mode", cl), ("frxd", cl), ("fryd", cl),
        ("frxo", cl), ("fryo", cl), ("time", cl), ("ifrm", cl), ("nfrm", cl),
        ("bptr", PT(c_ubyte)), ("cpal", PT(c_ubyte))]
    def intr(y, x, w, base, tran): base.paste(tran.crop((0, y, x, y + 1)), w)
    def skew(i, r): return r >> ((7 - (i & 2)) >> (1 + (i & 1)))
    def fill(w, d, p):
        retn = Image.new("L", d, w.bkgd) if (w.tran < 0) else \
               Image.new("RGBA", d)
        if (w.tran < 0):
            retn.putpalette(p)
        return retn
    def WriteFunc(d, w):
        cpal = string_at(w[0].cpal, w[0].clrs * 3)
        list = d.contents.value[0]
        if (len(list) == 0):
            list.append(Image.new("RGBA", (w[0].xdim, w[0].ydim)))
        tail = len(list) - 1
        base = Image.frombytes("L", (w[0].frxd, w[0].fryd),
                               string_at(w[0].bptr, w[0].frxd * w[0].fryd))
        if (w[0].intr != 0):
            tran = base.copy()
            [intr(skew(y, y) + (skew(y, w[0].fryd - 1) + 1, 0)[(y & 7) == 0],
                  w[0].frxd, (0, y), base, tran) for y in range(w[0].fryd)]
        tran = Image.eval(base, lambda indx: (255, 0)[indx == w[0].tran])
        base.putpalette(cpal)
        list[tail].paste(base, (w[0].frxo, w[0].fryo), tran)
        list[tail].info = {"delay" : w[0].time}
        if (w[0].ifrm != (w[0].nfrm - 1)):
            trgt = (tail, d.contents.value[1])[w[0].mode == 3]
            list.append(list[trgt].copy() if (trgt >= 0) else
                        fill(w[0], (w[0].xdim, w[0].ydim), cpal))
            if (w[0].mode != 3):
                d.contents.value[1] = w[0].ifrm
            if (w[0].mode == 2):
                list[tail + 1].paste(fill(w[0], (w[0].frxd, w[0].fryd), cpal),
                                                (w[0].frxo, w[0].fryo))
    try: file = open(file, "rb")
    except IOError: return []
    file.seek(0, 2)
    size = file.tell()
    file.seek(0, 0)
    list = [[], -1]
    CDLL(("%s.so", "%s.dll")[system() == "Windows"] % "./gif_load"). \
    GIF_Load(file.read(), size,
             CFUNCTYPE(None, PT(py_object), PT(GIF_WHDR))(WriteFunc),
             None, pointer(py_object(list)), 0)
    file.close()
    return list[0]

def GIF_Save(file, fext):
    list = GIF_Load("%s.gif" % file)
    [pict.save("%s_f%d.%s" % (file, indx, fext))
     for (indx, pict) in enumerate(list)]

GIF_Save("insert_gif_name_here_without_extension", "png")
```
