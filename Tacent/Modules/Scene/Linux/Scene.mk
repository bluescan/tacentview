##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=Scene
ConfigurationName      :=Release
WorkspacePath          :=/home/tristan/github/tacit-texview/Tacent/UnitTests/Linux
ProjectPath            :=/home/tristan/github/tacit-texview/Tacent/Modules/Scene/Linux
IntermediateDirectory  :=$(ConfigurationName)
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Tristan
Date                   :=20/04/20
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
ObjectsFileList        :="Scene.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch)../Inc $(IncludeSwitch)../../Foundation/Inc $(IncludeSwitch)../../Math/Inc $(IncludeSwitch)../../System/Inc $(IncludeSwitch). 
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
Objects0=$(IntermediateDirectory)/up_Src_tInstance.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tLodGroup.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tMaterial.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tObject.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tPath.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tSelection.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tPolyModel.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tLight.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tMesh.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tCamera.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/up_Src_tAttribute.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tSkeleton.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_tWorld.cpp$(ObjectSuffix) 



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
	@$(MakeDirCommand) "/home/tristan/github/tacit-texview/Tacent/UnitTests/Linux/.build-release"
	@echo rebuilt > "/home/tristan/github/tacit-texview/Tacent/UnitTests/Linux/.build-release/Scene"

MakeIntermediateDirs:
	@test -d $(ConfigurationName) || $(MakeDirCommand) $(ConfigurationName)


$(ConfigurationName):
	@test -d $(ConfigurationName) || $(MakeDirCommand) $(ConfigurationName)

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/up_Src_tInstance.cpp$(ObjectSuffix): ../Src/tInstance.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tInstance.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tInstance.cpp$(DependSuffix) -MM ../Src/tInstance.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Scene/Src/tInstance.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tInstance.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tInstance.cpp$(PreprocessSuffix): ../Src/tInstance.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tInstance.cpp$(PreprocessSuffix) ../Src/tInstance.cpp

$(IntermediateDirectory)/up_Src_tLodGroup.cpp$(ObjectSuffix): ../Src/tLodGroup.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tLodGroup.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tLodGroup.cpp$(DependSuffix) -MM ../Src/tLodGroup.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Scene/Src/tLodGroup.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tLodGroup.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tLodGroup.cpp$(PreprocessSuffix): ../Src/tLodGroup.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tLodGroup.cpp$(PreprocessSuffix) ../Src/tLodGroup.cpp

$(IntermediateDirectory)/up_Src_tMaterial.cpp$(ObjectSuffix): ../Src/tMaterial.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tMaterial.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tMaterial.cpp$(DependSuffix) -MM ../Src/tMaterial.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Scene/Src/tMaterial.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tMaterial.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tMaterial.cpp$(PreprocessSuffix): ../Src/tMaterial.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tMaterial.cpp$(PreprocessSuffix) ../Src/tMaterial.cpp

$(IntermediateDirectory)/up_Src_tObject.cpp$(ObjectSuffix): ../Src/tObject.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tObject.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tObject.cpp$(DependSuffix) -MM ../Src/tObject.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Scene/Src/tObject.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tObject.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tObject.cpp$(PreprocessSuffix): ../Src/tObject.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tObject.cpp$(PreprocessSuffix) ../Src/tObject.cpp

$(IntermediateDirectory)/up_Src_tPath.cpp$(ObjectSuffix): ../Src/tPath.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tPath.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tPath.cpp$(DependSuffix) -MM ../Src/tPath.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Scene/Src/tPath.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tPath.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tPath.cpp$(PreprocessSuffix): ../Src/tPath.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tPath.cpp$(PreprocessSuffix) ../Src/tPath.cpp

$(IntermediateDirectory)/up_Src_tSelection.cpp$(ObjectSuffix): ../Src/tSelection.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tSelection.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tSelection.cpp$(DependSuffix) -MM ../Src/tSelection.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Scene/Src/tSelection.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tSelection.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tSelection.cpp$(PreprocessSuffix): ../Src/tSelection.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tSelection.cpp$(PreprocessSuffix) ../Src/tSelection.cpp

$(IntermediateDirectory)/up_Src_tPolyModel.cpp$(ObjectSuffix): ../Src/tPolyModel.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tPolyModel.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tPolyModel.cpp$(DependSuffix) -MM ../Src/tPolyModel.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Scene/Src/tPolyModel.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tPolyModel.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tPolyModel.cpp$(PreprocessSuffix): ../Src/tPolyModel.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tPolyModel.cpp$(PreprocessSuffix) ../Src/tPolyModel.cpp

$(IntermediateDirectory)/up_Src_tLight.cpp$(ObjectSuffix): ../Src/tLight.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tLight.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tLight.cpp$(DependSuffix) -MM ../Src/tLight.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Scene/Src/tLight.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tLight.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tLight.cpp$(PreprocessSuffix): ../Src/tLight.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tLight.cpp$(PreprocessSuffix) ../Src/tLight.cpp

$(IntermediateDirectory)/up_Src_tMesh.cpp$(ObjectSuffix): ../Src/tMesh.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tMesh.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tMesh.cpp$(DependSuffix) -MM ../Src/tMesh.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Scene/Src/tMesh.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tMesh.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tMesh.cpp$(PreprocessSuffix): ../Src/tMesh.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tMesh.cpp$(PreprocessSuffix) ../Src/tMesh.cpp

$(IntermediateDirectory)/up_Src_tCamera.cpp$(ObjectSuffix): ../Src/tCamera.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tCamera.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tCamera.cpp$(DependSuffix) -MM ../Src/tCamera.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Scene/Src/tCamera.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tCamera.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tCamera.cpp$(PreprocessSuffix): ../Src/tCamera.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tCamera.cpp$(PreprocessSuffix) ../Src/tCamera.cpp

$(IntermediateDirectory)/up_Src_tAttribute.cpp$(ObjectSuffix): ../Src/tAttribute.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tAttribute.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tAttribute.cpp$(DependSuffix) -MM ../Src/tAttribute.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Scene/Src/tAttribute.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tAttribute.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tAttribute.cpp$(PreprocessSuffix): ../Src/tAttribute.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tAttribute.cpp$(PreprocessSuffix) ../Src/tAttribute.cpp

$(IntermediateDirectory)/up_Src_tSkeleton.cpp$(ObjectSuffix): ../Src/tSkeleton.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tSkeleton.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tSkeleton.cpp$(DependSuffix) -MM ../Src/tSkeleton.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Scene/Src/tSkeleton.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tSkeleton.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tSkeleton.cpp$(PreprocessSuffix): ../Src/tSkeleton.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tSkeleton.cpp$(PreprocessSuffix) ../Src/tSkeleton.cpp

$(IntermediateDirectory)/up_Src_tWorld.cpp$(ObjectSuffix): ../Src/tWorld.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_tWorld.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_tWorld.cpp$(DependSuffix) -MM ../Src/tWorld.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Modules/Scene/Src/tWorld.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_tWorld.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_tWorld.cpp$(PreprocessSuffix): ../Src/tWorld.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_tWorld.cpp$(PreprocessSuffix) ../Src/tWorld.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(ConfigurationName)/


