cmake_minimum_required(VERSION 3.7)
project(Game)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

option(MYGAME_VENDORED "Use vendored libraries" OFF)

set(CMAKE_FRAMEWORK_PATH /Library/Frameworks)

if(MYGAME_VENDORED)
    add_subdirectory(vendored/sdl EXCLUDE_FROM_ALL)
else()
    # 1. Look for a SDL2 package, 2. look for the SDL2 component and 3. fail if none can be found
    find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)

    # 1. Look for a SDL2 package, 2. Look for the SDL2maincomponent and 3. DO NOT fail when SDL2main is not available
    find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
endif()

# Create your game executable target as usual
FILE(GLOB_RECURSE MyCSources src/*.c)
add_executable(Game ${MyCSources})
target_include_directories(Game PRIVATE include)


# SDL2::SDL2main may or may not be available. It is e.g. required by Windows GUI applications
if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(Game PRIVATE SDL2::SDL2main)
endif()

# Link to the actual SDL2 library. SDL2::SDL2 is the shared SDL library, SDL2::SDL2-static is the static SDL libarary.
target_link_libraries(Game PRIVATE SDL2::SDL2 m)


add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:Game>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:Game>/shaders
)