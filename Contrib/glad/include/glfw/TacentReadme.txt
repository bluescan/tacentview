I used something like this to generate the VS sln and project:
cmake -G "Visual Studio 15 2017 Win64" -DBUILD_SHARED_LIBS=OFF -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF .

For Ubuntu something like
cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release ..
make

I did that twice, once for Release and once for Debug. You may need to apt install xorg-dev first.


