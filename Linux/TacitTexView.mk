##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=TacitTexView
ConfigurationName      :=Release
WorkspaceConfiguration := $(ConfigurationName)
WorkspacePath          :=/home/tristan/github/tacit-texview/Linux
ProjectPath            :=/home/tristan/github/tacit-texview/Linux
IntermediateDirectory  :=./build-$(ConfigurationName)/
OutDir                 :=./build-$(ConfigurationName)/
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
OutputFile             :=./build-$(ConfigurationName)/bin/$(ProjectName)
Preprocessors          :=$(PreprocessorSwitch)PLATFORM_LINUX $(PreprocessorSwitch)CONFIG_RELEASE 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :=$(IntermediateDirectory)/ObjectsList.txt
PCHCompileFlags        :=
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
Objects0=./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_demo.cpp$(ObjectSuffix) ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_draw.cpp$(ObjectSuffix) ./build-$(ConfigurationName)//up_Src_TacitTexView.cpp$(ObjectSuffix) ./build-$(ConfigurationName)//up_Src_Crop.cpp$(ObjectSuffix) ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(ObjectSuffix) ./build-$(ConfigurationName)//up_Src_TacitImage.cpp$(ObjectSuffix) ./build-$(ConfigurationName)//up_Src_SaveDialogs.cpp$(ObjectSuffix) ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui.cpp$(ObjectSuffix) ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_widgets.cpp$(ObjectSuffix) ./build-$(ConfigurationName)//up_Src_Dialogs.cpp$(ObjectSuffix) \
	./build-$(ConfigurationName)//up_Src_ContentView.cpp$(ObjectSuffix) ./build-$(ConfigurationName)//up_Tacent_Contrib_glad_src_glad.c$(ObjectSuffix) ./build-$(ConfigurationName)//up_Src_Settings.cpp$(ObjectSuffix) ./build-$(ConfigurationName)//up_Src_ContactSheet.cpp$(ObjectSuffix) ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs $(OutputFile)

$(OutputFile): ./build-$(ConfigurationName)//.d "$(IntermediateDirectory)/System.relink" "$(IntermediateDirectory)/Foundation.relink" "$(IntermediateDirectory)/Math.relink" "$(IntermediateDirectory)/Image.relink" $(Objects) 
	@mkdir -p "./build-$(ConfigurationName)/"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

"$(IntermediateDirectory)/System.relink":
	@mkdir -p $(IntermediateDirectory)
	@echo stam > "$(IntermediateDirectory)/System.relink"


"$(IntermediateDirectory)/Foundation.relink":
	@mkdir -p $(IntermediateDirectory)
	@echo stam > "$(IntermediateDirectory)/Foundation.relink"


"$(IntermediateDirectory)/Math.relink":
	@mkdir -p $(IntermediateDirectory)
	@echo stam > "$(IntermediateDirectory)/Math.relink"


"$(IntermediateDirectory)/Image.relink":
	@mkdir -p $(IntermediateDirectory)
	@echo stam > "$(IntermediateDirectory)/Image.relink"




PostBuild:
	@echo Executing Post Build commands ...
	cp $(OutputFile) ..
	@echo Done

MakeIntermediateDirs:
	@mkdir -p "./build-$(ConfigurationName)/"
	@mkdir -p ""./build-$(ConfigurationName)/bin""

./build-$(ConfigurationName)//.d:
	@mkdir -p "./build-$(ConfigurationName)/"

PreBuild:


##
## Objects
##
./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_demo.cpp$(ObjectSuffix): ../Tacent/Contrib/imgui/imgui_demo.cpp ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_demo.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Contrib/imgui/imgui_demo.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_demo.cpp$(ObjectSuffix) $(IncludePath)
./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_demo.cpp$(DependSuffix): ../Tacent/Contrib/imgui/imgui_demo.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_demo.cpp$(ObjectSuffix) -MF./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_demo.cpp$(DependSuffix) -MM ../Tacent/Contrib/imgui/imgui_demo.cpp

./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_demo.cpp$(PreprocessSuffix): ../Tacent/Contrib/imgui/imgui_demo.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_demo.cpp$(PreprocessSuffix) ../Tacent/Contrib/imgui/imgui_demo.cpp

./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_draw.cpp$(ObjectSuffix): ../Tacent/Contrib/imgui/imgui_draw.cpp ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_draw.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Contrib/imgui/imgui_draw.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_draw.cpp$(ObjectSuffix) $(IncludePath)
./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_draw.cpp$(DependSuffix): ../Tacent/Contrib/imgui/imgui_draw.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_draw.cpp$(ObjectSuffix) -MF./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_draw.cpp$(DependSuffix) -MM ../Tacent/Contrib/imgui/imgui_draw.cpp

./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_draw.cpp$(PreprocessSuffix): ../Tacent/Contrib/imgui/imgui_draw.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_draw.cpp$(PreprocessSuffix) ../Tacent/Contrib/imgui/imgui_draw.cpp

./build-$(ConfigurationName)//up_Src_TacitTexView.cpp$(ObjectSuffix): ../Src/TacitTexView.cpp ./build-$(ConfigurationName)//up_Src_TacitTexView.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/TacitTexView.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_TacitTexView.cpp$(ObjectSuffix) $(IncludePath)
./build-$(ConfigurationName)//up_Src_TacitTexView.cpp$(DependSuffix): ../Src/TacitTexView.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT./build-$(ConfigurationName)//up_Src_TacitTexView.cpp$(ObjectSuffix) -MF./build-$(ConfigurationName)//up_Src_TacitTexView.cpp$(DependSuffix) -MM ../Src/TacitTexView.cpp

./build-$(ConfigurationName)//up_Src_TacitTexView.cpp$(PreprocessSuffix): ../Src/TacitTexView.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ./build-$(ConfigurationName)//up_Src_TacitTexView.cpp$(PreprocessSuffix) ../Src/TacitTexView.cpp

./build-$(ConfigurationName)//up_Src_Crop.cpp$(ObjectSuffix): ../Src/Crop.cpp ./build-$(ConfigurationName)//up_Src_Crop.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/Crop.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_Crop.cpp$(ObjectSuffix) $(IncludePath)
./build-$(ConfigurationName)//up_Src_Crop.cpp$(DependSuffix): ../Src/Crop.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT./build-$(ConfigurationName)//up_Src_Crop.cpp$(ObjectSuffix) -MF./build-$(ConfigurationName)//up_Src_Crop.cpp$(DependSuffix) -MM ../Src/Crop.cpp

./build-$(ConfigurationName)//up_Src_Crop.cpp$(PreprocessSuffix): ../Src/Crop.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ./build-$(ConfigurationName)//up_Src_Crop.cpp$(PreprocessSuffix) ../Src/Crop.cpp

./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(ObjectSuffix): ../Tacent/Contrib/imgui/examples/imgui_impl_opengl2.cpp ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Contrib/imgui/examples/imgui_impl_opengl2.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(ObjectSuffix) $(IncludePath)
./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(DependSuffix): ../Tacent/Contrib/imgui/examples/imgui_impl_opengl2.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(ObjectSuffix) -MF./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(DependSuffix) -MM ../Tacent/Contrib/imgui/examples/imgui_impl_opengl2.cpp

./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(PreprocessSuffix): ../Tacent/Contrib/imgui/examples/imgui_impl_opengl2.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_opengl2.cpp$(PreprocessSuffix) ../Tacent/Contrib/imgui/examples/imgui_impl_opengl2.cpp

./build-$(ConfigurationName)//up_Src_TacitImage.cpp$(ObjectSuffix): ../Src/TacitImage.cpp ./build-$(ConfigurationName)//up_Src_TacitImage.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/TacitImage.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_TacitImage.cpp$(ObjectSuffix) $(IncludePath)
./build-$(ConfigurationName)//up_Src_TacitImage.cpp$(DependSuffix): ../Src/TacitImage.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT./build-$(ConfigurationName)//up_Src_TacitImage.cpp$(ObjectSuffix) -MF./build-$(ConfigurationName)//up_Src_TacitImage.cpp$(DependSuffix) -MM ../Src/TacitImage.cpp

./build-$(ConfigurationName)//up_Src_TacitImage.cpp$(PreprocessSuffix): ../Src/TacitImage.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ./build-$(ConfigurationName)//up_Src_TacitImage.cpp$(PreprocessSuffix) ../Src/TacitImage.cpp

./build-$(ConfigurationName)//up_Src_SaveDialogs.cpp$(ObjectSuffix): ../Src/SaveDialogs.cpp ./build-$(ConfigurationName)//up_Src_SaveDialogs.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/SaveDialogs.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_SaveDialogs.cpp$(ObjectSuffix) $(IncludePath)
./build-$(ConfigurationName)//up_Src_SaveDialogs.cpp$(DependSuffix): ../Src/SaveDialogs.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT./build-$(ConfigurationName)//up_Src_SaveDialogs.cpp$(ObjectSuffix) -MF./build-$(ConfigurationName)//up_Src_SaveDialogs.cpp$(DependSuffix) -MM ../Src/SaveDialogs.cpp

./build-$(ConfigurationName)//up_Src_SaveDialogs.cpp$(PreprocessSuffix): ../Src/SaveDialogs.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ./build-$(ConfigurationName)//up_Src_SaveDialogs.cpp$(PreprocessSuffix) ../Src/SaveDialogs.cpp

./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui.cpp$(ObjectSuffix): ../Tacent/Contrib/imgui/imgui.cpp ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Contrib/imgui/imgui.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui.cpp$(ObjectSuffix) $(IncludePath)
./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui.cpp$(DependSuffix): ../Tacent/Contrib/imgui/imgui.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui.cpp$(ObjectSuffix) -MF./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui.cpp$(DependSuffix) -MM ../Tacent/Contrib/imgui/imgui.cpp

./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui.cpp$(PreprocessSuffix): ../Tacent/Contrib/imgui/imgui.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui.cpp$(PreprocessSuffix) ../Tacent/Contrib/imgui/imgui.cpp

./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_widgets.cpp$(ObjectSuffix): ../Tacent/Contrib/imgui/imgui_widgets.cpp ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_widgets.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Contrib/imgui/imgui_widgets.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Tacent_Contrib_imgui_imgui_widgets.cpp$(ObjectSuffix) $(IncludePath)
./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_widgets.cpp$(DependSuffix): ../Tacent/Contrib/imgui/imgui_widgets.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_widgets.cpp$(ObjectSuffix) -MF./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_widgets.cpp$(DependSuffix) -MM ../Tacent/Contrib/imgui/imgui_widgets.cpp

./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_widgets.cpp$(PreprocessSuffix): ../Tacent/Contrib/imgui/imgui_widgets.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_imgui_widgets.cpp$(PreprocessSuffix) ../Tacent/Contrib/imgui/imgui_widgets.cpp

./build-$(ConfigurationName)//up_Src_Dialogs.cpp$(ObjectSuffix): ../Src/Dialogs.cpp ./build-$(ConfigurationName)//up_Src_Dialogs.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/Dialogs.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_Dialogs.cpp$(ObjectSuffix) $(IncludePath)
./build-$(ConfigurationName)//up_Src_Dialogs.cpp$(DependSuffix): ../Src/Dialogs.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT./build-$(ConfigurationName)//up_Src_Dialogs.cpp$(ObjectSuffix) -MF./build-$(ConfigurationName)//up_Src_Dialogs.cpp$(DependSuffix) -MM ../Src/Dialogs.cpp

./build-$(ConfigurationName)//up_Src_Dialogs.cpp$(PreprocessSuffix): ../Src/Dialogs.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ./build-$(ConfigurationName)//up_Src_Dialogs.cpp$(PreprocessSuffix) ../Src/Dialogs.cpp

./build-$(ConfigurationName)//up_Src_ContentView.cpp$(ObjectSuffix): ../Src/ContentView.cpp ./build-$(ConfigurationName)//up_Src_ContentView.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/ContentView.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_ContentView.cpp$(ObjectSuffix) $(IncludePath)
./build-$(ConfigurationName)//up_Src_ContentView.cpp$(DependSuffix): ../Src/ContentView.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT./build-$(ConfigurationName)//up_Src_ContentView.cpp$(ObjectSuffix) -MF./build-$(ConfigurationName)//up_Src_ContentView.cpp$(DependSuffix) -MM ../Src/ContentView.cpp

./build-$(ConfigurationName)//up_Src_ContentView.cpp$(PreprocessSuffix): ../Src/ContentView.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ./build-$(ConfigurationName)//up_Src_ContentView.cpp$(PreprocessSuffix) ../Src/ContentView.cpp

./build-$(ConfigurationName)//up_Tacent_Contrib_glad_src_glad.c$(ObjectSuffix): ../Tacent/Contrib/glad/src/glad.c ./build-$(ConfigurationName)//up_Tacent_Contrib_glad_src_glad.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Contrib/glad/src/glad.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Tacent_Contrib_glad_src_glad.c$(ObjectSuffix) $(IncludePath)
./build-$(ConfigurationName)//up_Tacent_Contrib_glad_src_glad.c$(DependSuffix): ../Tacent/Contrib/glad/src/glad.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT./build-$(ConfigurationName)//up_Tacent_Contrib_glad_src_glad.c$(ObjectSuffix) -MF./build-$(ConfigurationName)//up_Tacent_Contrib_glad_src_glad.c$(DependSuffix) -MM ../Tacent/Contrib/glad/src/glad.c

./build-$(ConfigurationName)//up_Tacent_Contrib_glad_src_glad.c$(PreprocessSuffix): ../Tacent/Contrib/glad/src/glad.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ./build-$(ConfigurationName)//up_Tacent_Contrib_glad_src_glad.c$(PreprocessSuffix) ../Tacent/Contrib/glad/src/glad.c

./build-$(ConfigurationName)//up_Src_Settings.cpp$(ObjectSuffix): ../Src/Settings.cpp ./build-$(ConfigurationName)//up_Src_Settings.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/Settings.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_Settings.cpp$(ObjectSuffix) $(IncludePath)
./build-$(ConfigurationName)//up_Src_Settings.cpp$(DependSuffix): ../Src/Settings.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT./build-$(ConfigurationName)//up_Src_Settings.cpp$(ObjectSuffix) -MF./build-$(ConfigurationName)//up_Src_Settings.cpp$(DependSuffix) -MM ../Src/Settings.cpp

./build-$(ConfigurationName)//up_Src_Settings.cpp$(PreprocessSuffix): ../Src/Settings.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ./build-$(ConfigurationName)//up_Src_Settings.cpp$(PreprocessSuffix) ../Src/Settings.cpp

./build-$(ConfigurationName)//up_Src_ContactSheet.cpp$(ObjectSuffix): ../Src/ContactSheet.cpp ./build-$(ConfigurationName)//up_Src_ContactSheet.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Src/ContactSheet.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Src_ContactSheet.cpp$(ObjectSuffix) $(IncludePath)
./build-$(ConfigurationName)//up_Src_ContactSheet.cpp$(DependSuffix): ../Src/ContactSheet.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT./build-$(ConfigurationName)//up_Src_ContactSheet.cpp$(ObjectSuffix) -MF./build-$(ConfigurationName)//up_Src_ContactSheet.cpp$(DependSuffix) -MM ../Src/ContactSheet.cpp

./build-$(ConfigurationName)//up_Src_ContactSheet.cpp$(PreprocessSuffix): ../Src/ContactSheet.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ./build-$(ConfigurationName)//up_Src_ContactSheet.cpp$(PreprocessSuffix) ../Src/ContactSheet.cpp

./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(ObjectSuffix): ../Tacent/Contrib/imgui/examples/imgui_impl_glfw.cpp ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tristan/github/tacit-texview/Tacent/Contrib/imgui/examples/imgui_impl_glfw.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(ObjectSuffix) $(IncludePath)
./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(DependSuffix): ../Tacent/Contrib/imgui/examples/imgui_impl_glfw.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(ObjectSuffix) -MF./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(DependSuffix) -MM ../Tacent/Contrib/imgui/examples/imgui_impl_glfw.cpp

./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(PreprocessSuffix): ../Tacent/Contrib/imgui/examples/imgui_impl_glfw.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ./build-$(ConfigurationName)//up_Tacent_Contrib_imgui_examples_imgui_impl_glfw.cpp$(PreprocessSuffix) ../Tacent/Contrib/imgui/examples/imgui_impl_glfw.cpp


-include ./build-$(ConfigurationName)///*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


