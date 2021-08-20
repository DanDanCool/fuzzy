cmake_minimum_required(VERSION 3.10)

project(fuzzy LANGUAGES C)

set(SOURCE_FILES src/fuzzy.c src/fuzzy.h src/platform.h)

# set the different names here because on linux cmake likes to prepend 'lib'
if(WIN32)
	list(APPEND SOURCE_FILES src/win32.c)
	set(SHARED_LIB_NAME libfuzzy)
endif()

if (UNIX)
	list(APPEND SOURCE_FILES src/linux.c)
	set(SHARED_LIB_NAME fuzzy)
endif()

add_library(${SHARED_LIB_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${SHARED_LIB_NAME} PROPERTIES PUBLIC_HEADER src/fuzzy.h)
set_target_properties(${SHARED_LIB_NAME} PROPERTIES LINKER_LANGUAGE C)
target_include_directories(${SHARED_LIB_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)

add_executable(main src/main.c)
target_link_libraries(main PRIVATE ${SHARED_LIB_NAME})

if(WIN32)
	add_compile_definitions(FZF_WIN32 WIN32_LEAN_AND_MEAN)
	set_target_properties(${SHARED_LIB_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

if(UNIX)
	add_compile_definitions(FZF_LINUX)
	target_compile_options(${SHARED_LIB_NAME} PRIVATE -pthread)
	target_link_libraries(${SHARED_LIB_NAME} pthread)

	if(CMAKE_BUILD_TYPE STREQUAL DEBUG)
		message(STATUS "building debug configuration...")
		target_compile_options(${SHARED_LIB_NAME} PRIVATE -g)
		target_compile_options(main PRIVATE -g)
	endif()

	if(CMAKE_BUILD_TYPE STREQUAL RELEASE)
		message(STATUS "building release configuration...")
		target_compile_options(${SHARED_LIB_NAME} PRIVATE -O3)
		target_compile_options(main PRIVATE -O3)
	endif()
endif()
