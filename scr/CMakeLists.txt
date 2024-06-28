# CMakeLists.txt
cmake_minimum_required(VERSION 3.0)
project(kNN_MOF_m)  # Set your project name here
enable_language(C)

# Add your source files here
set(SOURCE_FILES
    main.c
    Func_dataIO.c
    Func_Fragments.c
    Func_Initialize.c
    Func_Prepro.c
    Func_MD.c
    Func_Print.c
    Func_SSIM.c
    Func_kNN.c
    Func_Disaggregate.c
    Func_Solar.c
)


# Add the executable target
add_executable(kNN_MOF_m ${SOURCE_FILES})
# Link against the math library
target_link_libraries(kNN_MOF_m m)


## cmake -G "MinGW Makefiles" .
## mingw32-make
