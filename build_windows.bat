set bin_path=bin

if exist %bin_path% (
    rd /S /Q %bin_path%
)

call cmake -S . -B build -G "Visual Studio 17 2022" -A Win32 -DCMAKE_BUILD_TYPE=Debug
cd build
call cmake --build .
cd ..
