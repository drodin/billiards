if(ANDROID OR IOS)
hunter_config(
    SDL2
    VERSION 2.0.12-p0
    CMAKE_ARGS "HIDAPI=NO"
)
elseif(MINGW)
hunter_config(
    SDL2
    VERSION 2.0.12-p0
    CMAKE_ARGS "DIRECTX=NO"
)
endif()

hunter_config(gl4es GIT_SUBMODULE "3rdParty/gl4es")
