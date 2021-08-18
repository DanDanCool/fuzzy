cmake_minimum_required(VERSION 3.10)

project(fuzzy LANGUAGES C)

set(SOURCE_FILES src/fuzzy.c src/fuzzy.h src/filesystem.h)

# set the different names here because on linux cmake likes to prepend 'lib'
if(WIN32)
	list(APPEND SOURCE_FILES src/win32.c)
	set(SHARED_LIB_NAME libfuzzy)
endif()

if (UNIX)
	list(APPEND SOURCE_FILES src/linux.c)
	set(SHARED_LIB_NAME fuzzy)
	add_compile_options(-g)
endif()

add_library(${SHARED_LIB_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${SHARED_LIB_NAME} PROPERTIES PUBLIC_HEADER src/fuzzy.h)
target_include_directories(${SHARED_LIB_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)

if(WIN32)
	set_target_properties(${SHARED_LIB_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

add_executable(main src/main.c)
target_link_libraries(main PRIVATE ${SHARED_LIB_NAME})

set(CMAKE_BUILD_TYPE Debug)
set_target_properties(${SHARED_LIB_NAME} PROPERTIES LINKER_LANGUAGE C)
