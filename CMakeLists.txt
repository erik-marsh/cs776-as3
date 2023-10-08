project(cs776-as2 CXX)

add_executable(as3 encoding.cpp main.cpp)
target_compile_features(as3 PRIVATE cxx_std_20)

add_executable(test-encoding encoding.cpp tests/test-encoding.cpp)
target_compile_features(test-encoding PRIVATE cxx_std_20)
