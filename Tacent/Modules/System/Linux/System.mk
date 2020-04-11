##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=System
ConfigurationName      :=Release
WorkspaceConfiguration := $(ConfigurationName)
WorkspacePath          :=/home/tristan/github/tacit-texview/Linux
ProjectPath            :=/home/tristan/github/tacit-texview/Tacent/Modules/System/Linux
IntermediateDirectory  :=../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux
OutDir                 :=../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux
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
Objects0=../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tScript.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTask.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tMachine.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tFile.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tCommand.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTime.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tPrint.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tRegex.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tThrow.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tChunk.cpp$(ObjectSuffix) \
	



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/$(OutputFile)

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/$(OutputFile): $(Objects)
	@mkdir -p "../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(AR) $(ArchiveOutputSwitch)$(OutputFile) @$(ObjectsFileList)
	@echo rebuilt > $(IntermediateDirectory)/System.relink

MakeIntermediateDirs:
	@mkdir -p "../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux"
	@mkdir -p ""../../../../Linux/build-$(ConfigurationName)/lib""

$(ConfigurationName):
	@mkdir -p "../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux"

PreBuild:


##
## Objects
##
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tScript.cpp$(ObjectSuffix): ../Src/tScript.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tScript.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tScript.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tScript.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tScript.cpp$(DependSuffix): ../Src/tScript.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tScript.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tScript.cpp$(DependSuffix) -MM ../Src/tScript.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tScript.cpp$(PreprocessSuffix): ../Src/tScript.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tScript.cpp$(PreprocessSuffix) ../Src/tScript.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTask.cpp$(ObjectSuffix): ../Src/tTask.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTask.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tTask.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tTask.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTask.cpp$(DependSuffix): ../Src/tTask.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTask.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTask.cpp$(DependSuffix) -MM ../Src/tTask.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTask.cpp$(PreprocessSuffix): ../Src/tTask.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTask.cpp$(PreprocessSuffix) ../Src/tTask.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tMachine.cpp$(ObjectSuffix): ../Src/tMachine.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tMachine.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tMachine.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tMachine.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tMachine.cpp$(DependSuffix): ../Src/tMachine.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tMachine.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tMachine.cpp$(DependSuffix) -MM ../Src/tMachine.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tMachine.cpp$(PreprocessSuffix): ../Src/tMachine.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tMachine.cpp$(PreprocessSuffix) ../Src/tMachine.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tFile.cpp$(ObjectSuffix): ../Src/tFile.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tFile.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tFile.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tFile.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tFile.cpp$(DependSuffix): ../Src/tFile.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tFile.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tFile.cpp$(DependSuffix) -MM ../Src/tFile.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tFile.cpp$(PreprocessSuffix): ../Src/tFile.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tFile.cpp$(PreprocessSuffix) ../Src/tFile.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tCommand.cpp$(ObjectSuffix): ../Src/tCommand.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tCommand.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tCommand.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tCommand.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tCommand.cpp$(DependSuffix): ../Src/tCommand.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tCommand.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tCommand.cpp$(DependSuffix) -MM ../Src/tCommand.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tCommand.cpp$(PreprocessSuffix): ../Src/tCommand.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tCommand.cpp$(PreprocessSuffix) ../Src/tCommand.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTime.cpp$(ObjectSuffix): ../Src/tTime.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTime.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tTime.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tTime.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTime.cpp$(DependSuffix): ../Src/tTime.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTime.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTime.cpp$(DependSuffix) -MM ../Src/tTime.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTime.cpp$(PreprocessSuffix): ../Src/tTime.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tTime.cpp$(PreprocessSuffix) ../Src/tTime.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tPrint.cpp$(ObjectSuffix): ../Src/tPrint.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tPrint.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tPrint.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tPrint.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tPrint.cpp$(DependSuffix): ../Src/tPrint.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tPrint.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tPrint.cpp$(DependSuffix) -MM ../Src/tPrint.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tPrint.cpp$(PreprocessSuffix): ../Src/tPrint.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tPrint.cpp$(PreprocessSuffix) ../Src/tPrint.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tRegex.cpp$(ObjectSuffix): ../Src/tRegex.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tRegex.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tRegex.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tRegex.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tRegex.cpp$(DependSuffix): ../Src/tRegex.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tRegex.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tRegex.cpp$(DependSuffix) -MM ../Src/tRegex.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tRegex.cpp$(PreprocessSuffix): ../Src/tRegex.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tRegex.cpp$(PreprocessSuffix) ../Src/tRegex.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tThrow.cpp$(ObjectSuffix): ../Src/tThrow.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tThrow.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tThrow.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tThrow.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tThrow.cpp$(DependSuffix): ../Src/tThrow.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tThrow.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tThrow.cpp$(DependSuffix) -MM ../Src/tThrow.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tThrow.cpp$(PreprocessSuffix): ../Src/tThrow.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tThrow.cpp$(PreprocessSuffix) ../Src/tThrow.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tChunk.cpp$(ObjectSuffix): ../Src/tChunk.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tChunk.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/System/Src/tChunk.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tChunk.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tChunk.cpp$(DependSuffix): ../Src/tChunk.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tChunk.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tChunk.cpp$(DependSuffix) -MM ../Src/tChunk.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tChunk.cpp$(PreprocessSuffix): ../Src/tChunk.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux/up_Src_tChunk.cpp$(PreprocessSuffix) ../Src/tChunk.cpp


-include ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/System/Linux//*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


