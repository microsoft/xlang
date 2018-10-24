clang.exe main.cpp ^
 -o base.exe ^
 -I ..\strings ^
 -std=c++17 ^
 -fuse-ld=lld ^
 -fno-delayed-template-parsing ^
 -fno-ms-compatibility ^
 -Xclang -flto-visibility-public-std
