##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=Math
ConfigurationName      :=Release
WorkspaceConfiguration := $(ConfigurationName)
WorkspacePath          :=/home/tristan/github/tacit-texview/Linux
ProjectPath            :=/home/tristan/github/tacit-texview/Tacent/Modules/Math/Linux
IntermediateDirectory  :=../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux
OutDir                 :=../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux
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
IncludePath            :=  $(IncludeSwitch)../Inc $(IncludeSwitch)../../Foundation/Inc $(IncludeSwitch). 
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
Objects0=../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tLinearAlgebra.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tSpline.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tRandom.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tHash.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tColour.cpp$(ObjectSuffix) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tGeometry.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/$(OutputFile)

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/$(OutputFile): $(Objects)
	@mkdir -p "../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(AR) $(ArchiveOutputSwitch)$(OutputFile) @$(ObjectsFileList)
	@echo rebuilt > $(IntermediateDirectory)/Math.relink

MakeIntermediateDirs:
	@mkdir -p "../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux"
	@mkdir -p ""../../../../Linux/build-$(ConfigurationName)/lib""

$(ConfigurationName):
	@mkdir -p "../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux"

PreBuild:


##
## Objects
##
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tLinearAlgebra.cpp$(ObjectSuffix): ../Src/tLinearAlgebra.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tLinearAlgebra.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Math/Src/tLinearAlgebra.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tLinearAlgebra.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tLinearAlgebra.cpp$(DependSuffix): ../Src/tLinearAlgebra.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tLinearAlgebra.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tLinearAlgebra.cpp$(DependSuffix) -MM ../Src/tLinearAlgebra.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tLinearAlgebra.cpp$(PreprocessSuffix): ../Src/tLinearAlgebra.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tLinearAlgebra.cpp$(PreprocessSuffix) ../Src/tLinearAlgebra.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tSpline.cpp$(ObjectSuffix): ../Src/tSpline.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tSpline.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Math/Src/tSpline.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tSpline.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tSpline.cpp$(DependSuffix): ../Src/tSpline.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tSpline.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tSpline.cpp$(DependSuffix) -MM ../Src/tSpline.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tSpline.cpp$(PreprocessSuffix): ../Src/tSpline.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tSpline.cpp$(PreprocessSuffix) ../Src/tSpline.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tRandom.cpp$(ObjectSuffix): ../Src/tRandom.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tRandom.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Math/Src/tRandom.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tRandom.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tRandom.cpp$(DependSuffix): ../Src/tRandom.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tRandom.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tRandom.cpp$(DependSuffix) -MM ../Src/tRandom.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tRandom.cpp$(PreprocessSuffix): ../Src/tRandom.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tRandom.cpp$(PreprocessSuffix) ../Src/tRandom.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tHash.cpp$(ObjectSuffix): ../Src/tHash.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tHash.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Math/Src/tHash.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tHash.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tHash.cpp$(DependSuffix): ../Src/tHash.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tHash.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tHash.cpp$(DependSuffix) -MM ../Src/tHash.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tHash.cpp$(PreprocessSuffix): ../Src/tHash.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tHash.cpp$(PreprocessSuffix) ../Src/tHash.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tColour.cpp$(ObjectSuffix): ../Src/tColour.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tColour.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Math/Src/tColour.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tColour.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tColour.cpp$(DependSuffix): ../Src/tColour.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tColour.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tColour.cpp$(DependSuffix) -MM ../Src/tColour.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tColour.cpp$(PreprocessSuffix): ../Src/tColour.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tColour.cpp$(PreprocessSuffix) ../Src/tColour.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tGeometry.cpp$(ObjectSuffix): ../Src/tGeometry.cpp ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tGeometry.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Math/Src/tGeometry.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tGeometry.cpp$(ObjectSuffix) $(IncludePath)
../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tGeometry.cpp$(DependSuffix): ../Src/tGeometry.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tGeometry.cpp$(ObjectSuffix) -MF../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tGeometry.cpp$(DependSuffix) -MM ../Src/tGeometry.cpp

../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tGeometry.cpp$(PreprocessSuffix): ../Src/tGeometry.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux/up_Src_tGeometry.cpp$(PreprocessSuffix) ../Src/tGeometry.cpp


-include ../../../../Linux/build-$(ConfigurationName)/__/Tacent/Modules/Math/Linux//*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


