add_executable(dicom_file_reader_validation
    dicom_file_reader_validation.cpp
)

target_compile_features(dicom_file_reader_validation PRIVATE cxx_std_17)
target_link_libraries(dicom_file_reader_validation PRIVATE vna_dicom)

add_test(NAME dicom_file_reader_validation
         COMMAND dicom_file_reader_validation)

add_executable(send_config_validation
    send_config_validation.cpp
)

target_compile_features(send_config_validation PRIVATE cxx_std_17)
target_link_libraries(send_config_validation PRIVATE vna_config)

add_test(NAME send_config_validation
         COMMAND send_config_validation)

add_executable(local_instance_storage_validation
    local_instance_storage_validation.cpp
)

target_compile_features(local_instance_storage_validation PRIVATE cxx_std_17)
target_link_libraries(local_instance_storage_validation PRIVATE vna_storage)

add_test(NAME local_instance_storage_validation
         COMMAND local_instance_storage_validation)

add_executable(dicom_uid_validation
    dicom_uid_validation.cpp
)

target_compile_features(dicom_uid_validation PRIVATE cxx_std_17)
target_link_libraries(dicom_uid_validation PRIVATE vna_domain)

add_test(NAME dicom_uid_validation
         COMMAND dicom_uid_validation)

find_program(DUMP2DCM_EXECUTABLE dump2dcm)
if(NOT DUMP2DCM_EXECUTABLE)
    message(FATAL_ERROR
        "dump2dcm (DCMTK CLI tool) is required to generate the positive "
        "DicomFileReader test fixture, but it was not found on PATH. "
        "Install the DCMTK CLI tools (for example, 'brew install dcmtk' on "
        "macOS) and re-run CMake.")
endif()

set(POSITIVE_FIXTURE_DUMP
    "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/positive_instance.dump.txt")
set(POSITIVE_FIXTURE_DCM
    "${CMAKE_CURRENT_BINARY_DIR}/positive_instance.dcm")

add_custom_command(
    OUTPUT "${POSITIVE_FIXTURE_DCM}"
    COMMAND "${DUMP2DCM_EXECUTABLE}" +g
            "${POSITIVE_FIXTURE_DUMP}"
            "${POSITIVE_FIXTURE_DCM}"
    DEPENDS "${POSITIVE_FIXTURE_DUMP}"
    COMMENT "Generating positive DicomFileReader fixture via dump2dcm"
    VERBATIM
)
add_custom_target(positive_instance_dcm
    DEPENDS "${POSITIVE_FIXTURE_DCM}")

add_executable(dicom_file_reader_positive
    dicom_file_reader_positive.cpp
)
target_compile_features(dicom_file_reader_positive PRIVATE cxx_std_17)
target_link_libraries(dicom_file_reader_positive PRIVATE vna_dicom)
add_dependencies(dicom_file_reader_positive positive_instance_dcm)

add_test(NAME dicom_file_reader_positive
         COMMAND dicom_file_reader_positive "${POSITIVE_FIXTURE_DCM}")
