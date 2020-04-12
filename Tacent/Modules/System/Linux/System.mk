##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=System
ConfigurationName      :=Release
WorkspacePath          :=/home/tristan/github/tacit-texview/Linux
ProjectPath            :=/home/tristan/github/tacit-texview/Tacent/Modules/System/Linux
IntermediateDirectory  :=$(ConfigurationName)
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Tristan
Date                   :=12/04/20
CodeLitePath           :=/home/tristan/.codelite
LinkerName             :=/usr/bin/clang++
SharedObjectLinkerName :=/usr/bin/clang++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/lib$(ProjectName).a
Preprocessors          :=$(PreprocessorSwitch)PLATFORM_LINUX $(PreprocessorSwitch)CONFIG_RELEASE 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="System.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch)../Inc $(IncludeSwitch)../../Foundation/Inc $(IncludeSwitch)../../Math/Inc 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/llvm-ar rcu
CXX      := /usr/bin/clang++
CC       := /usr/bin/clang
CXXFLAGS := -Wno-switch -std=c++17 -O2 $(Preprocessors)
CFLAGS   :=  -O2 $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/llvm-as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/up_Src_tScript.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tTask.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tMachine.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tFile.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tCommand.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tTime.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tPrint.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tRegex.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tThrow.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tChunk.cpp$(ObjectSuffix) \
	



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(IntermediateDirectory) $(OutputFile)

$(OutputFile): $(Objects)
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(AR) $(ArchiveOutputSwitch)$(OutputFile) @$(ObjectsFileList)
	@$(MakeDirCommand) "/home/tristan/github/tacit-texview/Linux/.build-release"
	@echo rebuilt > "/home/tristan/github/tacit-texview/Linux/.build-release/System"

MakeIntermediateDirs:
	@test -d $(ConfigurationName) || $(MakeDirCommand) $(ConfigurationName)


$(ConfigurationName):
	@test -d $(ConfigurationName) || $(MakeDirCommand) $(ConfigurationName)

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/up_Src_tScript.cpp$(ObjectSuffix): ../Src/tScript.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tScript.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tScript.cpp$(DependSuffix) -MM ../Src/tScript.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tScript.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tScript.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tScript.cpp$(PreprocessSuffix): ../Src/tScript.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tScript.cpp$(PreprocessSuffix) ../Src/tScript.cpp

$(IntermediateDirectory)/up_Src_tTask.cpp$(ObjectSuffix): ../Src/tTask.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tTask.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tTask.cpp$(DependSuffix) -MM ../Src/tTask.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tTask.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tTask.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tTask.cpp$(PreprocessSuffix): ../Src/tTask.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tTask.cpp$(PreprocessSuffix) ../Src/tTask.cpp

$(IntermediateDirectory)/up_Src_tMachine.cpp$(ObjectSuffix): ../Src/tMachine.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tMachine.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tMachine.cpp$(DependSuffix) -MM ../Src/tMachine.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tMachine.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tMachine.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tMachine.cpp$(PreprocessSuffix): ../Src/tMachine.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tMachine.cpp$(PreprocessSuffix) ../Src/tMachine.cpp

$(IntermediateDirectory)/up_Src_tFile.cpp$(ObjectSuffix): ../Src/tFile.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tFile.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tFile.cpp$(DependSuffix) -MM ../Src/tFile.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tFile.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tFile.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tFile.cpp$(PreprocessSuffix): ../Src/tFile.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tFile.cpp$(PreprocessSuffix) ../Src/tFile.cpp

$(IntermediateDirectory)/up_Src_tCommand.cpp$(ObjectSuffix): ../Src/tCommand.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tCommand.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tCommand.cpp$(DependSuffix) -MM ../Src/tCommand.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tCommand.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tCommand.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tCommand.cpp$(PreprocessSuffix): ../Src/tCommand.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tCommand.cpp$(PreprocessSuffix) ../Src/tCommand.cpp

$(IntermediateDirectory)/up_Src_tTime.cpp$(ObjectSuffix): ../Src/tTime.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tTime.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tTime.cpp$(DependSuffix) -MM ../Src/tTime.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tTime.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tTime.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tTime.cpp$(PreprocessSuffix): ../Src/tTime.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tTime.cpp$(PreprocessSuffix) ../Src/tTime.cpp

$(IntermediateDirectory)/up_Src_tPrint.cpp$(ObjectSuffix): ../Src/tPrint.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tPrint.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tPrint.cpp$(DependSuffix) -MM ../Src/tPrint.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tPrint.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tPrint.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tPrint.cpp$(PreprocessSuffix): ../Src/tPrint.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tPrint.cpp$(PreprocessSuffix) ../Src/tPrint.cpp

$(IntermediateDirectory)/up_Src_tRegex.cpp$(ObjectSuffix): ../Src/tRegex.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tRegex.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tRegex.cpp$(DependSuffix) -MM ../Src/tRegex.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tRegex.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tRegex.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tRegex.cpp$(PreprocessSuffix): ../Src/tRegex.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tRegex.cpp$(PreprocessSuffix) ../Src/tRegex.cpp

$(IntermediateDirectory)/up_Src_tThrow.cpp$(ObjectSuffix): ../Src/tThrow.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tThrow.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tThrow.cpp$(DependSuffix) -MM ../Src/tThrow.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tThrow.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tThrow.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tThrow.cpp$(PreprocessSuffix): ../Src/tThrow.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tThrow.cpp$(PreprocessSuffix) ../Src/tThrow.cpp

$(IntermediateDirectory)/up_Src_tChunk.cpp$(ObjectSuffix): ../Src/tChunk.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tChunk.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tChunk.cpp$(DependSuffix) -MM ../Src/tChunk.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tChunk.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tChunk.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tChunk.cpp$(PreprocessSuffix): ../Src/tChunk.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tChunk.cpp$(PreprocessSuffix) ../Src/tChunk.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(ConfigurationName)/


