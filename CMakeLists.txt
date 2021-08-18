cmake_minimum_required(VERSION 3.10)

project(fuzzy LANGUAGES C)

set(SOURCE_FILES src/fuzzy.c src/fuzzy.h src/filesystem.h)

if(WIN32)
	list(APPEND SOURCE_FILES src/win32.c)
endif()

if (UNIX)
	list(APPEND SOURCE_FILES src/linux.c)
	add_compile_options(-g)
endif()

add_library(libfuzzy SHARED ${SOURCE_FILES})
set_target_properties(libfuzzy PROPERTIES PUBLIC_HEADER src/fuzzy.h)
target_include_directories(libfuzzy PRIVATE ${PROJECT_SOURCE_DIR}/src)

if(WIN32)
	set_target_properties(libfuzzy PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

add_executable(main src/main.c)
target_link_libraries(main PUBLIC libfuzzy)

set(CMAKE_BUILD_TYPE Debug)
set_target_properties(libfuzzy PROPERTIES LINKER_LANGUAGE C)
