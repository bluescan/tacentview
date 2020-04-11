##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=Foundation
ConfigurationName      :=Release
WorkspacePath          :=/home/tristan/github/tacit-texview/Linux
ProjectPath            :=/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Linux
IntermediateDirectory  :=$(ConfigurationName)
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Tristan
Date                   :=11/04/20
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
ObjectsFileList        :="Foundation.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch)../Inc 
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
Objects0=$(IntermediateDirectory)/up_Src_tAssert.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tBitArray.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tMemory.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tString.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tPlatform.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tPool.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tUnits.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tStandard.cpp$(ObjectSuffix) 



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
	@echo rebuilt > "/home/tristan/github/tacit-texview/Linux/.build-release/Foundation"

MakeIntermediateDirs:
	@test -d $(ConfigurationName) || $(MakeDirCommand) $(ConfigurationName)


$(ConfigurationName):
	@test -d $(ConfigurationName) || $(MakeDirCommand) $(ConfigurationName)

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/up_Src_tAssert.cpp$(ObjectSuffix): ../Src/tAssert.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tAssert.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tAssert.cpp$(DependSuffix) -MM ../Src/tAssert.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tAssert.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tAssert.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tAssert.cpp$(PreprocessSuffix): ../Src/tAssert.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tAssert.cpp$(PreprocessSuffix) ../Src/tAssert.cpp

$(IntermediateDirectory)/up_Src_tBitArray.cpp$(ObjectSuffix): ../Src/tBitArray.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tBitArray.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tBitArray.cpp$(DependSuffix) -MM ../Src/tBitArray.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tBitArray.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tBitArray.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tBitArray.cpp$(PreprocessSuffix): ../Src/tBitArray.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tBitArray.cpp$(PreprocessSuffix) ../Src/tBitArray.cpp

$(IntermediateDirectory)/up_Src_tMemory.cpp$(ObjectSuffix): ../Src/tMemory.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tMemory.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tMemory.cpp$(DependSuffix) -MM ../Src/tMemory.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tMemory.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tMemory.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tMemory.cpp$(PreprocessSuffix): ../Src/tMemory.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tMemory.cpp$(PreprocessSuffix) ../Src/tMemory.cpp

$(IntermediateDirectory)/up_Src_tString.cpp$(ObjectSuffix): ../Src/tString.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tString.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tString.cpp$(DependSuffix) -MM ../Src/tString.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tString.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tString.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tString.cpp$(PreprocessSuffix): ../Src/tString.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tString.cpp$(PreprocessSuffix) ../Src/tString.cpp

$(IntermediateDirectory)/up_Src_tPlatform.cpp$(ObjectSuffix): ../Src/tPlatform.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tPlatform.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tPlatform.cpp$(DependSuffix) -MM ../Src/tPlatform.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tPlatform.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tPlatform.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tPlatform.cpp$(PreprocessSuffix): ../Src/tPlatform.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tPlatform.cpp$(PreprocessSuffix) ../Src/tPlatform.cpp

$(IntermediateDirectory)/up_Src_tPool.cpp$(ObjectSuffix): ../Src/tPool.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tPool.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tPool.cpp$(DependSuffix) -MM ../Src/tPool.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tPool.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tPool.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tPool.cpp$(PreprocessSuffix): ../Src/tPool.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tPool.cpp$(PreprocessSuffix) ../Src/tPool.cpp

$(IntermediateDirectory)/up_Src_tUnits.cpp$(ObjectSuffix): ../Src/tUnits.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tUnits.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tUnits.cpp$(DependSuffix) -MM ../Src/tUnits.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tUnits.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tUnits.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tUnits.cpp$(PreprocessSuffix): ../Src/tUnits.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tUnits.cpp$(PreprocessSuffix) ../Src/tUnits.cpp

$(IntermediateDirectory)/up_Src_tStandard.cpp$(ObjectSuffix): ../Src/tStandard.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tStandard.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tStandard.cpp$(DependSuffix) -MM ../Src/tStandard.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tStandard.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tStandard.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tStandard.cpp$(PreprocessSuffix): ../Src/tStandard.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tStandard.cpp$(PreprocessSuffix) ../Src/tStandard.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(ConfigurationName)/


