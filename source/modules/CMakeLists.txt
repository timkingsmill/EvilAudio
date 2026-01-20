set(CMAKE_FOLDER modules)

option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" OFF)

juce_add_modules(
    ALIAS_NAMESPACE evilaudio
    evilaudio_core
    evilaudio_eq
    evilaudio_lookandfeel
)


