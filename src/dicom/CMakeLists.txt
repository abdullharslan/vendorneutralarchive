find_package(DCMTK REQUIRED CONFIG)

add_library(vna_dicom STATIC
    DicomFileReader.cpp
    EchoScu.cpp
    StoreScp.cpp
)

target_include_directories(vna_dicom
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
)

target_compile_features(vna_dicom PUBLIC cxx_std_17)

target_link_libraries(vna_dicom
    PUBLIC  vna_domain vna_storage
    PRIVATE DCMTK::dcmdata DCMTK::dcmnet
)
