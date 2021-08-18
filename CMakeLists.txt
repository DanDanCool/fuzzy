cmake_minimum_required(VERSION 3.10)

project(fuzzy)

#add_library(${PROJECT_NAME} SHARED src/fuzzy.c src/fuzzy.h)
#set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER src/fuzzy.h)
#target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
#
#add_executable(main src/main.c)
#target_link_libraries(main PUBLIC ${PROJECT_NAME})
#target_link_directories(main PRIVATE src)

add_executable(${PROJECT_NAME} src/main.c)

set(CMAKE_BUILD_TYPE Debug)
set_target_properties(fuzzy PROPERTIES LINKER_LANGUAGE C)
add_compile_options(-g)
