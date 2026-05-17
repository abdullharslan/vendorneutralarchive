add_library(vna_domain STATIC
    AeTitle.cpp
    DicomStatus.cpp
    DicomNode.cpp
    DicomFile.cpp
    DicomInstanceMetadata.cpp
    EchoResult.cpp
    StoreResult.cpp
)

target_include_directories(vna_domain
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
)

target_compile_features(vna_domain PUBLIC cxx_std_17)
