set bin_path=bin

if exist %bin_path% (
    rd /S /Q %bin_path%
)

call cmake -S . -B build -G "Visual Studio 16 2019" -A Win32 -DCMAKE_BUILD_TYPE=Debug -DTEST=true
cd build
call cmake --build .
cd ..
