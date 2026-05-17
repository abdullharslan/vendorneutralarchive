add_library(vna_config STATIC
    CliParsing.cpp
    SendConfig.cpp
    ReceiverConfig.cpp
)

target_include_directories(vna_config
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
)

target_compile_features(vna_config PUBLIC cxx_std_17)
target_link_libraries(vna_config PUBLIC vna_domain)
