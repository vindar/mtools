REM Clean up the /build sub-directory
copy .\build\.gitignore build-gitignore.saved
copy .\build\cmake_mtools.py cmake_mtools.py.saved
rmdir /s /q ".\build\"
mkdir build
move build-gitignore.saved .\build\.gitignore
move cmake_mtools.py.saved .\build\cmake_mtools.py
