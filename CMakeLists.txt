####################################################################################################
cmake_minimum_required(VERSION 3.2)

set(CMAKE_BUILD_TYPE Debug)

project(PicoScheme VERSION 0.1 LANGUAGES CXX)
set(LIB_NAME picoscheme)

# Enable c++17 for MS-compiler
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif(MSVC)

# Set debug flags for gcc
if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb  -Wno-padded")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb  -Wno-padded")
endif(CMAKE_COMPILER_IS_GNUCC)

# Add source directory
file(GLOB SOURCES "src/*.cpp")

set(INCLUDE_PATH "${PROJECT_SOURCE_DIR}/src/include/picoscm")

####################################################################################################
option (BUILD_SHARED_LIBS "build picoscheme as shared library" OFF)

add_library(${LIB_NAME} ${SOURCES})

target_include_directories(${LIB_NAME} PRIVATE ${INCLUDE_PATH})
target_compile_options(${LIB_NAME} PRIVATE -Wall) # -Wno-sign-conversion)
set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)

install (TARGETS ${LIB_NAME} DESTINATION lib)
install (DIRECTORY ${INCLUDE_PATH} DESTINATION include FILES_MATCHING PATTERN "*.hpp")

####################################################################################################
option (GENERATE_TESTMAIN "Generate a test main program with a basic scheme repl" ON)

if (GENERATE_TESTMAIN)
    set(EXEC_NAME picoscm)

    add_executable(${EXEC_NAME} "test/${EXEC_NAME}.cpp")
    target_include_directories(${EXEC_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/src/include")
    target_link_libraries(${EXEC_NAME} ${LIB_NAME})
    target_compile_options(${EXEC_NAME} PRIVATE -Wall -Wno-padded)
    set_target_properties(${EXEC_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)

    install (TARGETS ${EXEC_NAME} DESTINATION bin)
endif (GENERATE_TESTMAIN)
