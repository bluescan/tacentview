##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=UnitTests
ConfigurationName      :=Debug
WorkspacePath          :=/home/tristan/github/tacent/UnitTests/Linux
ProjectPath            :=/home/tristan/github/tacent/UnitTests/Linux
IntermediateDirectory  :=$(ConfigurationName)
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Tristan
Date                   :=30/04/20
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
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=$(PreprocessorSwitch)PLATFORM_LINUX $(PreprocessorSwitch)CONFIG_DEBUG 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="UnitTests.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            := -pthread 
IncludePath            :=  $(IncludeSwitch)../../Modules/Foundation/Inc $(IncludeSwitch)../../Modules/Math/Inc $(IncludeSwitch)../../Modules/System/Inc $(IncludeSwitch)../../Modules/Image/Inc $(IncludeSwitch)../../Modules/Scene/Inc 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)Image $(LibrarySwitch)System $(LibrarySwitch)Math $(LibrarySwitch)Foundation 
ArLibs                 :=  "Image" "System" "Math" "Foundation" 
LibPath                := $(LibraryPathSwitch)../../Modules/Image/Linux/Debug $(LibraryPathSwitch)../../Modules/System/Linux/Debug $(LibraryPathSwitch)../../Modules/Math/Linux/Debug $(LibraryPathSwitch)../../Modules/Foundation/Linux/Debug 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/llvm-ar rcu
CXX      := /usr/bin/clang++
CC       := /usr/bin/clang
CXXFLAGS := -Wno-switch -std=c++17 -g $(Preprocessors)
CFLAGS   :=  -g $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/llvm-as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/up_Src_TestFoundation.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_TestImage.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_UnitTests.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_TestMath.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_TestSystem.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_TestScene.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d ".build-debug/Foundation" ".build-debug/Math" ".build-debug/System" ".build-debug/Image" $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

".build-debug/Foundation":
	@$(MakeDirCommand) ".build-debug"
	@echo stam > ".build-debug/Foundation"


".build-debug/Math":
	@$(MakeDirCommand) ".build-debug"
	@echo stam > ".build-debug/Math"


".build-debug/System":
	@$(MakeDirCommand) ".build-debug"
	@echo stam > ".build-debug/System"


".build-debug/Image":
	@$(MakeDirCommand) ".build-debug"
	@echo stam > ".build-debug/Image"




PostBuild:
	@echo Executing Post Build commands ...
	cp Debug/UnitTests /home/tristan/github/tacent/UnitTests/Linux/../UnitTests
	
	
	@echo Done

MakeIntermediateDirs:
	@test -d $(ConfigurationName) || $(MakeDirCommand) $(ConfigurationName)


$(IntermediateDirectory)/.d:
	@test -d $(ConfigurationName) || $(MakeDirCommand) $(ConfigurationName)

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/up_Src_TestFoundation.cpp$(ObjectSuffix): ../Src/TestFoundation.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_TestFoundation.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_TestFoundation.cpp$(DependSuffix) -MM ../Src/TestFoundation.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacent/UnitTests/Src/TestFoundation.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_TestFoundation.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_TestFoundation.cpp$(PreprocessSuffix): ../Src/TestFoundation.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_TestFoundation.cpp$(PreprocessSuffix) ../Src/TestFoundation.cpp

$(IntermediateDirectory)/up_Src_TestImage.cpp$(ObjectSuffix): ../Src/TestImage.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_TestImage.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_TestImage.cpp$(DependSuffix) -MM ../Src/TestImage.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacent/UnitTests/Src/TestImage.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_TestImage.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_TestImage.cpp$(PreprocessSuffix): ../Src/TestImage.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_TestImage.cpp$(PreprocessSuffix) ../Src/TestImage.cpp

$(IntermediateDirectory)/up_Src_UnitTests.cpp$(ObjectSuffix): ../Src/UnitTests.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_UnitTests.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_UnitTests.cpp$(DependSuffix) -MM ../Src/UnitTests.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacent/UnitTests/Src/UnitTests.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_UnitTests.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_UnitTests.cpp$(PreprocessSuffix): ../Src/UnitTests.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_UnitTests.cpp$(PreprocessSuffix) ../Src/UnitTests.cpp

$(IntermediateDirectory)/up_Src_TestMath.cpp$(ObjectSuffix): ../Src/TestMath.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_TestMath.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_TestMath.cpp$(DependSuffix) -MM ../Src/TestMath.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacent/UnitTests/Src/TestMath.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_TestMath.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_TestMath.cpp$(PreprocessSuffix): ../Src/TestMath.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_TestMath.cpp$(PreprocessSuffix) ../Src/TestMath.cpp

$(IntermediateDirectory)/up_Src_TestSystem.cpp$(ObjectSuffix): ../Src/TestSystem.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_TestSystem.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_TestSystem.cpp$(DependSuffix) -MM ../Src/TestSystem.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacent/UnitTests/Src/TestSystem.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_TestSystem.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_TestSystem.cpp$(PreprocessSuffix): ../Src/TestSystem.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_TestSystem.cpp$(PreprocessSuffix) ../Src/TestSystem.cpp

$(IntermediateDirectory)/up_Src_TestScene.cpp$(ObjectSuffix): ../Src/TestScene.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_TestScene.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_TestScene.cpp$(DependSuffix) -MM ../Src/TestScene.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacent/UnitTests/Src/TestScene.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_TestScene.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_TestScene.cpp$(PreprocessSuffix): ../Src/TestScene.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_TestScene.cpp$(PreprocessSuffix) ../Src/TestScene.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(ConfigurationName)/


