##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=Foundation
ConfigurationName      :=Release
WorkspaceConfiguration := $(ConfigurationName)
WorkspacePath          :=/home/tristan/github/tacit-texview/Linux
ProjectPath            :=/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Linux
IntermediateDirectory  :=../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux
OutDir                 :=../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Tristan
Date                   :=04/10/20
CodeLitePath           :=
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
OutputFile             :=../../../../Linux/build-$(ConfigurationName)/lib/lib$(ProjectName).a
Preprocessors          :=$(PreprocessorSwitch)PLATFORM_LINUX $(PreprocessorSwitch)CONFIG_RELEASE 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :=$(IntermediateDirectory)/ObjectsList.txt
PCHCompileFlags        :=
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
Objects0=../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tAssert.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tBitArray.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tMemory.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tString.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPlatform.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPool.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tUnits.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tStandard.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/$(OutputFile)

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/$(OutputFile): $(Objects)
	@mkdir -p "../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(AR) $(ArchiveOutputSwitch)$(OutputFile) @$(ObjectsFileList)
	@echo rebuilt > $(IntermediateDirectory)/Foundation.relink

MakeIntermediateDirs:
	@mkdir -p "../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux"
	@mkdir -p ""../../../../Linux/build-$(ConfigurationName)/lib""

$(ConfigurationName):
	@mkdir -p "../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux"

PreBuild:


##
## Objects
##
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tAssert.cpp$(ObjectSuffix): ../Src/tAssert.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tAssert.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tAssert.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tAssert.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tAssert.cpp$(DependSuffix): ../Src/tAssert.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tAssert.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tAssert.cpp$(DependSuffix) -MM ../Src/tAssert.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tAssert.cpp$(PreprocessSuffix): ../Src/tAssert.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tAssert.cpp$(PreprocessSuffix) ../Src/tAssert.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tBitArray.cpp$(ObjectSuffix): ../Src/tBitArray.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tBitArray.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tBitArray.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tBitArray.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tBitArray.cpp$(DependSuffix): ../Src/tBitArray.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tBitArray.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tBitArray.cpp$(DependSuffix) -MM ../Src/tBitArray.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tBitArray.cpp$(PreprocessSuffix): ../Src/tBitArray.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tBitArray.cpp$(PreprocessSuffix) ../Src/tBitArray.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tMemory.cpp$(ObjectSuffix): ../Src/tMemory.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tMemory.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tMemory.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tMemory.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tMemory.cpp$(DependSuffix): ../Src/tMemory.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tMemory.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tMemory.cpp$(DependSuffix) -MM ../Src/tMemory.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tMemory.cpp$(PreprocessSuffix): ../Src/tMemory.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tMemory.cpp$(PreprocessSuffix) ../Src/tMemory.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tString.cpp$(ObjectSuffix): ../Src/tString.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tString.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tString.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tString.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tString.cpp$(DependSuffix): ../Src/tString.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tString.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tString.cpp$(DependSuffix) -MM ../Src/tString.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tString.cpp$(PreprocessSuffix): ../Src/tString.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tString.cpp$(PreprocessSuffix) ../Src/tString.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPlatform.cpp$(ObjectSuffix): ../Src/tPlatform.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPlatform.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tPlatform.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tPlatform.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPlatform.cpp$(DependSuffix): ../Src/tPlatform.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPlatform.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPlatform.cpp$(DependSuffix) -MM ../Src/tPlatform.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPlatform.cpp$(PreprocessSuffix): ../Src/tPlatform.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPlatform.cpp$(PreprocessSuffix) ../Src/tPlatform.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPool.cpp$(ObjectSuffix): ../Src/tPool.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPool.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tPool.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tPool.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPool.cpp$(DependSuffix): ../Src/tPool.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPool.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPool.cpp$(DependSuffix) -MM ../Src/tPool.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPool.cpp$(PreprocessSuffix): ../Src/tPool.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tPool.cpp$(PreprocessSuffix) ../Src/tPool.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tUnits.cpp$(ObjectSuffix): ../Src/tUnits.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tUnits.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tUnits.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tUnits.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tUnits.cpp$(DependSuffix): ../Src/tUnits.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tUnits.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tUnits.cpp$(DependSuffix) -MM ../Src/tUnits.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tUnits.cpp$(PreprocessSuffix): ../Src/tUnits.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tUnits.cpp$(PreprocessSuffix) ../Src/tUnits.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tStandard.cpp$(ObjectSuffix): ../Src/tStandard.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tStandard.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Foundation/Src/tStandard.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tStandard.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tStandard.cpp$(DependSuffix): ../Src/tStandard.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tStandard.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tStandard.cpp$(DependSuffix) -MM ../Src/tStandard.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tStandard.cpp$(PreprocessSuffix): ../Src/tStandard.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux/up_Src_tStandard.cpp$(PreprocessSuffix) ../Src/tStandard.cpp


-include ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Foundation/Linux//*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


