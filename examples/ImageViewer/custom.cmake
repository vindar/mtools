
# custom build for ImageViewer: use WinMain() instead of main() on windows
    
if (WIN32)
    add_executable(${projectname} WIN32 ${projectdir}/main.cpp)        
else ()
    add_executable(${projectname} ${projectdir}/main.cpp)        
endif ()

        
