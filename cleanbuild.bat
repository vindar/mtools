REM Clean up the /build sub-directory
copy .\build\.gitignore build-gitignore.saved
rmdir /s /q ".\build\"
mkdir build
move build-gitignore.saved .\build\.gitignore
