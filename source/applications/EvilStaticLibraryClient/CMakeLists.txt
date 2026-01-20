add_executable(EvilStaticLibraryClient
    source/EvilStaticLibraryClient.cpp
)

target_link_libraries(EvilStaticLibraryClient
    PRIVATE 
        evil_juce_core
)