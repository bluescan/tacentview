Windows GLFW lib build:
	Open x64 Native Tools Command Prompt
	mkdir buildwin
	cd buildwin
	cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_MSVC_RUNTILE_LIBRARY_DLL=False -G"NMake Makefiles"
	nmake

Linux GLFW lib build:
	sudo apt-get install xorg-dev
	mkdir builddir
	cd builddir
	cmake .. -DCMAKE_BUILD_TYPE=Release
	make
