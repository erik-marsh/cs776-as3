project(cs776-as2 CXX)
add_executable(as2 encoding.cpp dejong.cpp main.cpp)
add_executable(test-encoding encoding.cpp tests/test-encoding.cpp)
add_executable(test-dejong-range dejong.cpp tests/test-dejong-range.cpp)