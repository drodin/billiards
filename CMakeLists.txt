cmake_minimum_required(VERSION 3.9.2)

option(ALT_CONTROLS "Use alternative controls" ON)

# For Android Studio we will force generated toolchain
if(ANDROID_STUDIO)
    set(gen_toolchain "${CMAKE_CURRENT_BINARY_DIR}/generated/toolchain.cmake")
    configure_file(
            "${CMAKE_CURRENT_LIST_DIR}/cmake/template/toolchain.cmake.in"
            "${gen_toolchain}"
            @ONLY
    )
    set(CMAKE_TOOLCHAIN_FILE "${gen_toolchain}" CACHE PATH "" FORCE)
endif()

option(CMAKE_VERBOSE_MAKEFILE "Verbose makefile" OFF)

option(HUNTER_KEEP_PACKAGE_SOURCES "Keep third party sources" ON)
option(HUNTER_STATUS_DEBUG "Print debug info" OFF)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.315.tar.gz"
    SHA1 "ea5d72afc0df67f2126e7fd069f20c5f723709e1"
    LOCAL
)

project(billiards DESCRIPTION "Pyramid Billiards" VERSION 3.43.1)

if(ANDROID OR IOS)
    hunter_add_package(glu)
    find_package(glu CONFIG REQUIRED)
    set(OPENGL_LIBRARIES glu::GLU)

    add_definitions(-D__MOBILE__=1)
    add_definitions(-DTOUCH=1)
    add_definitions(-DWETAB=1)
else()
    find_package(OpenGL MODULE REQUIRED)
endif()

hunter_add_package(SDL2)
hunter_add_package(SDL_mixer)
hunter_add_package(SDL_net)
hunter_add_package(PNG)
hunter_add_package(freetype)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL_mixer CONFIG REQUIRED)
find_package(SDL_net CONFIG REQUIRED)
find_package(PNG CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)

set(SRC
    src/billard3d.h
    src/billard3d.c
    src/language.c
    src/language.h
    src/billmove.c
    src/billmove.h
    src/billard.c
    src/billard.h
    src/vmath.c
    src/vmath.h
    src/ball.c
    src/ball.h
    src/png_loader.c
    src/png_loader.h
    src/table.c
    src/table.h
    src/queue.c
    src/queue.h
    src/aiplayer.c
    src/aiplayer.h
    src/options.c
    src/options.h
    src/evaluate_move.c
    src/evaluate_move.h
    src/helpscreen.c
    src/helpscreen.h
    src/textobj.c
    src/textobj.h
    src/font.c
    src/font.h
    src/sys_stuff.c
    src/sys_stuff.h
    src/net_socket.c
    src/net_socket.h
    src/sound_stuff.c
    src/sound_stuff.h
    src/menu.c
    src/menu.h
    src/player.h
    src/room.c
    src/room.h
    src/mesh.h
    src/mesh.c
    src/barchair.h
    src/bartable.h
    src/burlap_sofa.h
    src/fireplace.h
    src/fire.h
    src/fire.c
    src/getopt_long.h
    src/getopt_long.c
)

if(ALT_CONTROLS)
    add_definitions(-DALT_CONTROLS=1)
endif()

if(ANDROID)
    add_library(${PROJECT_NAME} SHARED ${SRC})

    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "")
elseif(APPLE)

if(IOS)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SRC} data apple/ios/Assets.xcassets apple/ios/LaunchScreen.storyboard)

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/cmake/template/iOSBundleInfo.plist.in"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
    )

    set_source_files_properties(apple/ios/Assets.xcassets PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_source_files_properties(apple/ios/LaunchScreen.storyboard PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
else()
    list(APPEND SRC osx/config.m)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SRC} data apple/macos/Assets.xcassets apple/macos/app.entitlements)

    set(MACOSX_BUNDLE_APP_CATEGORY "public.app-category.sports-games")

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/cmake/template/MacOSXBundleInfo.plist.in"
        MACOSX_BUNDLE_COPYRIGHT "Copyright © 2021 Dmitry Rodin. All rights reserved."
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Mac Developer"
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "apple/macos/app.entitlements"
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_DESCRIPTION}"
    )

    set_source_files_properties(apple/macos/Assets.xcassets PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

    add_definitions(-DGL_SILENCE_DEPRECATION=1)

    set_target_properties(${PROJECT_NAME} PROPERTIES
        RESOURCE data
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION_PATCH}"
        XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
        XCODE_ATTRIBUTE_SKIP_INSTALL "No"
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "$ENV{CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}"
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.drodin.${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_DESCRIPTION}"
    )

else()
    add_executable(${PROJECT_NAME} ${SRC})
endif()

add_definitions(-DUSE_SOUND=1)
add_definitions(-DNETWORKING=1)
#add_definitions(-DVMATH_SINGLE_PRECISION=1)
#add_definitions(-DFAST_MATH=1)
#add_definitions(-DUSE_BINDBUFFER=1)

add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")
add_definitions(-DPROJECT_DESCRIPTION="${PROJECT_DESCRIPTION}")

include_directories(
    src
    ${OPENGL_INCLUDE_DIR}
)
set(LIBS
    ${OPENGL_LIBRARIES}
    SDL2::SDL2
    SDL_mixer::SDL_mixer
    SDL_net::SDL_net
    PNG::PNG
    freetype::freetype
)

if(MINGW)

add_definitions(-DUSE_WIN=1)

    LIST(PREPEND LIBS
        mingw32
        SDL2::SDL2main
    )
    LIST(APPEND LIBS
        wsock32
        iphlpapi
    )
endif()

target_link_libraries(${PROJECT_NAME} ${LIBS})
