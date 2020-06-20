cmake_minimum_required(VERSION 3.5)

# Set project name and force C language
project(adarch C)

# Compile in "bin" folder
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Compile flags (specific for OS and/or compilers)
if(MSVC)
    add_definitions(/D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

# Select "src" folder for current OS
if(WIN32)
    set(src_path "${CMAKE_SOURCE_DIR}/windows/src/")
else()
    set(src_path "${CMAKE_SOURCE_DIR}/linux/src/")
endif()

message(STATUS "adarch_src_path=${src_path}")

# Create ADARCH executable (all source files directly in it)
add_executable(adarch 
    ${src_path}adarch.c
    ${src_path}config.c
    ${src_path}config.h
    ${src_path}connection.c
    ${src_path}connection.h
    ${src_path}connection_active.c
    ${src_path}connection_active.h
    ${src_path}connection_pool.c
    ${src_path}connection_pool.h
    ${src_path}connection_task.c
    ${src_path}connection_task.h
    ${src_path}jsmn.c
    ${src_path}jsmn.h
    ${src_path}list.h
    ${src_path}log.c
    ${src_path}log.h
    ${src_path}python_connection.c
    ${src_path}python_connection.h
    ${src_path}python_launcher.c
    ${src_path}python_launcher.h
    ${src_path}python_log.c
    ${src_path}python_log.h
    ${src_path}python_utils.c
    ${src_path}python_utils.h
    ${src_path}stdafx.h
    ${src_path}threadpool.c
    ${src_path}threadpool.h
    ${src_path}utils.c
    ${src_path}utils.h
    )

# Find Threads library and link it
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
message(STATUS "CMAKE_THREAD_LIBS_INIT=${CMAKE_THREAD_LIBS_INIT}")
message(STATUS "CMAKE_USE_SPROC_INIT=${CMAKE_USE_SPROC_INIT}")
message(STATUS "CMAKE_USE_WIN32_THREADS_INIT=${CMAKE_USE_WIN32_THREADS_INIT}")
message(STATUS "CMAKE_USE_PTHREADS_INIT=${CMAKE_USE_PTHREADS_INIT}")
message(STATUS "CMAKE_HP_PTHREADS_INIT=${CMAKE_HP_PTHREADS_INIT}")

target_link_libraries(adarch Threads::Threads)

# Find Python v3.5 or upper and link it
find_package(PythonInterp 3.5 REQUIRED)
message(STATUS "PYTHONINTERP_FOUND=${PYTHONINTERP_FOUND}")
message(STATUS "PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}")
message(STATUS "PYTHON_VERSION_STRING=${PYTHON_VERSION_STRING}")

find_package(PythonLibs 3.5 REQUIRED)
message(STATUS "PYTHONLIBS_FOUND=${PYTHONLIBS_FOUND}")
message(STATUS "PYTHON_LIBRARIES=${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_INCLUDE_PATH=${PYTHON_INCLUDE_PATH}")
message(STATUS "PYTHON_INCLUDE_DIRS=${PYTHON_INCLUDE_DIRS}")
message(STATUS "PYTHON_DEBUG_LIBRARIES=${PYTHON_DEBUG_LIBRARIES}")
message(STATUS "PYTHONLIBS_VERSION_STRING=${PYTHONLIBS_VERSION_STRING}")

include_directories(${PYTHON_INCLUDE_DIRS})
target_link_libraries(adarch ${PYTHON_LIBRARIES})
