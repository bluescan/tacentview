##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=TacitTexView
ConfigurationName      :=Release
WorkspacePath          :=/home/tristan/github/tacit-texview/Linux
ProjectPath            :=/home/tristan/github/tacit-texview/Linux
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
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=$(PreprocessorSwitch)PLATFORM_LINUX $(PreprocessorSwitch)GLFW_INCLUDE_NONE $(PreprocessorSwitch)CONFIG_RELEASE 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="TacitTexView.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            := -pthread 
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)../Tacent/Modules/System/Inc $(IncludeSwitch)../Tacent/Modules/Foundation/Inc $(IncludeSwitch)../Tacent/Modules/Math/Inc $(IncludeSwitch)../Tacent/Modules/Image/Inc $(IncludeSwitch)../Tacent/Contrib/imgui $(IncludeSwitch)../Tacent/Contrib/imgui/examples $(IncludeSwitch)../Tacent/Contrib/glad/include $(IncludeSwitch)../Tacent/Contrib/glfw/Linux/include $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)Image $(LibrarySwitch)System $(LibrarySwitch)Math $(LibrarySwitch)Foundation $(LibrarySwitch)glfw3 $(LibrarySwitch)dl 
ArLibs                 :=  "Image" "System" "Math" "Foundation" "glfw3" "dl" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../Tacent/Modules/Image/Linux/Release $(LibraryPathSwitch)../Tacent/Modules/System/Linux/Release $(LibraryPathSwitch)../Tacent/Modules/Math/Linux/Release $(LibraryPathSwitch)../Tacent/Modules/Foundation/Linux/Release $(LibraryPathSwitch)../Tacent/Contrib/glfw/Linux/Release 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/llvm-ar rcu
CXX      := /usr/bin/clang++
CC       := /usr/bin/clang
CXXFLAGS := -Wno-switch -std=c++17 -Wno-format-security -O2 $(Preprocessors)
CFLAGS   :=  -O2 $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/llvm-as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_demo.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_draw.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_TacitTexView.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_Crop.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_TacitImage.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_SaveDialogs.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_widgets.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_Dialogs.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/up_Src_ContentView.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Tacent_Contrib_glad_src_glad.c$(ObjectSuffix) $(IntermediateDirectory)/up_Src_Settings.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Src_ContactSheet.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d ".build-release/System" ".build-release/Foundation" ".build-release/Math" ".build-release/Image" $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

".build-release/System":
	@$(MakeDirCommand) ".build-release"
	@echo stam > ".build-release/System"


".build-release/Foundation":
	@$(MakeDirCommand) ".build-release"
	@echo stam > ".build-release/Foundation"


".build-release/Math":
	@$(MakeDirCommand) ".build-release"
	@echo stam > ".build-release/Math"


".build-release/Image":
	@$(MakeDirCommand) ".build-release"
	@echo stam > ".build-release/Image"




PostBuild:
	@echo Executing Post Build commands ...
	cp Release/TacitTexView ..
	./create_deb.sh
	@echo Done

MakeIntermediateDirs:
	@test -d $(ConfigurationName) || $(MakeDirCommand) $(ConfigurationName)


$(IntermediateDirectory)/.d:
	@test -d $(ConfigurationName) || $(MakeDirCommand) $(ConfigurationName)

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_demo.cpp$(ObjectSuffix): ../Tacent/Contrib/imgui/imgui_demo.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_demo.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_demo.cpp$(DependSuffix) -MM ../Tacent/Contrib/imgui/imgui_demo.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Contrib/imgui/imgui_demo.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_demo.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_demo.cpp$(PreprocessSuffix): ../Tacent/Contrib/imgui/imgui_demo.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_demo.cpp$(PreprocessSuffix) ../Tacent/Contrib/imgui/imgui_demo.cpp

$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_draw.cpp$(ObjectSuffix): ../Tacent/Contrib/imgui/imgui_draw.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_draw.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_draw.cpp$(DependSuffix) -MM ../Tacent/Contrib/imgui/imgui_draw.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Contrib/imgui/imgui_draw.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_draw.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_draw.cpp$(PreprocessSuffix): ../Tacent/Contrib/imgui/imgui_draw.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_draw.cpp$(PreprocessSuffix) ../Tacent/Contrib/imgui/imgui_draw.cpp

$(IntermediateDirectory)/up_Src_TacitTexView.cpp$(ObjectSuffix): ../Src/TacitTexView.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_TacitTexView.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_TacitTexView.cpp$(DependSuffix) -MM ../Src/TacitTexView.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/TacitTexView.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_TacitTexView.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_TacitTexView.cpp$(PreprocessSuffix): ../Src/TacitTexView.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_TacitTexView.cpp$(PreprocessSuffix) ../Src/TacitTexView.cpp

$(IntermediateDirectory)/up_Src_Crop.cpp$(ObjectSuffix): ../Src/Crop.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_Crop.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_Crop.cpp$(DependSuffix) -MM ../Src/Crop.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/Crop.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_Crop.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_Crop.cpp$(PreprocessSuffix): ../Src/Crop.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_Crop.cpp$(PreprocessSuffix) ../Src/Crop.cpp

$(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(ObjectSuffix): ../Tacent/Contrib/imgui/examples/imgui_impl_opengl2.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(DependSuffix) -MM ../Tacent/Contrib/imgui/examples/imgui_impl_opengl2.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Contrib/imgui/examples/imgui_impl_opengl2.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(PreprocessSuffix): ../Tacent/Contrib/imgui/examples/imgui_impl_opengl2.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(PreprocessSuffix) ../Tacent/Contrib/imgui/examples/imgui_impl_opengl2.cpp

$(IntermediateDirectory)/up_Src_TacitImage.cpp$(ObjectSuffix): ../Src/TacitImage.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_TacitImage.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_TacitImage.cpp$(DependSuffix) -MM ../Src/TacitImage.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/TacitImage.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_TacitImage.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_TacitImage.cpp$(PreprocessSuffix): ../Src/TacitImage.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_TacitImage.cpp$(PreprocessSuffix) ../Src/TacitImage.cpp

$(IntermediateDirectory)/up_Src_SaveDialogs.cpp$(ObjectSuffix): ../Src/SaveDialogs.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_SaveDialogs.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_SaveDialogs.cpp$(DependSuffix) -MM ../Src/SaveDialogs.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/SaveDialogs.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_SaveDialogs.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_SaveDialogs.cpp$(PreprocessSuffix): ../Src/SaveDialogs.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_SaveDialogs.cpp$(PreprocessSuffix) ../Src/SaveDialogs.cpp

$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui.cpp$(ObjectSuffix): ../Tacent/Contrib/imgui/imgui.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui.cpp$(DependSuffix) -MM ../Tacent/Contrib/imgui/imgui.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Contrib/imgui/imgui.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui.cpp$(PreprocessSuffix): ../Tacent/Contrib/imgui/imgui.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui.cpp$(PreprocessSuffix) ../Tacent/Contrib/imgui/imgui.cpp

$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_widgets.cpp$(ObjectSuffix): ../Tacent/Contrib/imgui/imgui_widgets.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_widgets.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_widgets.cpp$(DependSuffix) -MM ../Tacent/Contrib/imgui/imgui_widgets.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Contrib/imgui/imgui_widgets.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_widgets.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_widgets.cpp$(PreprocessSuffix): ../Tacent/Contrib/imgui/imgui_widgets.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_widgets.cpp$(PreprocessSuffix) ../Tacent/Contrib/imgui/imgui_widgets.cpp

$(IntermediateDirectory)/up_Src_Dialogs.cpp$(ObjectSuffix): ../Src/Dialogs.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_Dialogs.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_Dialogs.cpp$(DependSuffix) -MM ../Src/Dialogs.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/Dialogs.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_Dialogs.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_Dialogs.cpp$(PreprocessSuffix): ../Src/Dialogs.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_Dialogs.cpp$(PreprocessSuffix) ../Src/Dialogs.cpp

$(IntermediateDirectory)/up_Src_ContentView.cpp$(ObjectSuffix): ../Src/ContentView.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_ContentView.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_ContentView.cpp$(DependSuffix) -MM ../Src/ContentView.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/ContentView.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_ContentView.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_ContentView.cpp$(PreprocessSuffix): ../Src/ContentView.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_ContentView.cpp$(PreprocessSuffix) ../Src/ContentView.cpp

$(IntermediateDirectory)/up_Tacent_Contrib_glad_src_glad.c$(ObjectSuffix): ../Tacent/Contrib/glad/src/glad.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Tacent_Contrib_glad_src_glad.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Tacent_Contrib_glad_src_glad.c$(DependSuffix) -MM ../Tacent/Contrib/glad/src/glad.c
	$(CC) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Contrib/glad/src/glad.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Tacent_Contrib_glad_src_glad.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Tacent_Contrib_glad_src_glad.c$(PreprocessSuffix): ../Tacent/Contrib/glad/src/glad.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Tacent_Contrib_glad_src_glad.c$(PreprocessSuffix) ../Tacent/Contrib/glad/src/glad.c

$(IntermediateDirectory)/up_Src_Settings.cpp$(ObjectSuffix): ../Src/Settings.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_Settings.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_Settings.cpp$(DependSuffix) -MM ../Src/Settings.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/Settings.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_Settings.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_Settings.cpp$(PreprocessSuffix): ../Src/Settings.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_Settings.cpp$(PreprocessSuffix) ../Src/Settings.cpp

$(IntermediateDirectory)/up_Src_ContactSheet.cpp$(ObjectSuffix): ../Src/ContactSheet.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Src_ContactSheet.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Src_ContactSheet.cpp$(DependSuffix) -MM ../Src/ContactSheet.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/ContactSheet.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_ContactSheet.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Src_ContactSheet.cpp$(PreprocessSuffix): ../Src/ContactSheet.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Src_ContactSheet.cpp$(PreprocessSuffix) ../Src/ContactSheet.cpp

$(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(ObjectSuffix): ../Tacent/Contrib/imgui/examples/imgui_impl_glfw.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(DependSuffix) -MM ../Tacent/Contrib/imgui/examples/imgui_impl_glfw.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Contrib/imgui/examples/imgui_impl_glfw.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(PreprocessSuffix): ../Tacent/Contrib/imgui/examples/imgui_impl_glfw.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(PreprocessSuffix) ../Tacent/Contrib/imgui/examples/imgui_impl_glfw.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(ConfigurationName)/


