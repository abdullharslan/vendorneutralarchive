add_executable(vna-send main.cpp)
target_compile_features(vna-send PRIVATE cxx_std_17)
target_link_libraries(vna-send PRIVATE vna_config vna_dicom)
