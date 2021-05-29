cmake_minimum_required(VERSION 3.16.3)

project(PacGame)

message(${CMAKE_SYSTEM_NAME})
message(${CMAKE_CURRENT_SOURCE_DIR})

# if(MINGW)
#     set(CMAKE_SYSTEM_NAME Windows)  
#     set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc )
#     set(CMAKE_RANLIB x86_64-w64-mingw32-ranlib)
# endif()

message("${CMAKE_SYSTEM_NAME}")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message("in linux flow")
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

    include(FindSDL2_image)
    include(FindSDL2_ttf)

    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
# CROSS COMPILING
elseif(${CMAKE_SYSTEM_NAME} STREQUAL  "Windows")
    set( SDL2_INCLUDE_DIRS "/home/frog/repos/sdl-repos/sdl-pacmonster/include/SDL2" )
    set( SDL2_LIBRARIES "/home/frog/repos/sdl-repos/sdl-pacmonster/buildMingw64/SDL2.dll" )
    set( SDL2_IMAGE_LIBRARIES "/home/frog/repos/sdl-repos/sdl-pacmonster/buildMingw64/SDL2_image.dll" )
    set( SDL2_TTF_LIBRARIES "/home/frog/repos/sdl-repos/sdl-pacmonster/buildMingw64/SDL2_ttf.dll" )
    message("windows stuff")
endif()

message("after flow")
message("${SDL2_INCLUDE_DIRS}")
message("SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
message("SDL2_IMAGE_LIBRARIES: ${SDL2_IMAGE_LIBRARIES}")
message("SDL2_TTF_LIBRARIES: ${SDL2_TTF_LIBRARIES}")

include_directories(../include)
add_executable(PacGame pac_game_jumbo.c)
link_directories("/home/frog/repos/sdl-repos/sdl-pacmonster/buildMingw64")

target_link_libraries(PacGame "/usr/x86_64-w64-mingw32/lib/libmingw32.a" "/home/frog/repos/sdl-repos/sdl-pacmonster/buildMingw64/libSDL2main.a" ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES})

if(${CMAKE_SYSTEM_NAME} STREQUAL  "Windows")
    message("in windows library flow")
    target_link_libraries(PacGame -lmingw32)
endif()