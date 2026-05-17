add_library(vna_storage STATIC
    LocalInstanceStorage.cpp
)

target_include_directories(vna_storage
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
)

target_compile_features(vna_storage PUBLIC cxx_std_17)
