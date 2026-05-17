add_executable(vna-receiver main.cpp)
target_compile_features(vna-receiver PRIVATE cxx_std_17)
target_link_libraries(vna-receiver PRIVATE vna_config vna_dicom vna_storage)
