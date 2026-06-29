g++ screen_off.cpp -o screen_off_64.exe -m64 -static -O2 -march=x86-64 -mwindows -luser32 -lgdi32 -lole32 -lshell32

g++32 screen_off.cpp -o screen_off_32.exe -m32 -static -O2 -march=i686 -mwindows -luser32 -lgdi32 -lole32 -lshell32