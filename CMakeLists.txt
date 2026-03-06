# Yeu cau phien ban CMake toi thieu
cmake_minimum_required(VERSION 3.10)

# Ten du an va ngon ngu lap trinh (CXX = C++)
project(DMS_Core_Project VERSION 2.0)

# Thiet lap chuan C++14 (Bat buoc de ho tro tot thu vien da luong)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

# Tim kiem thu vien OpenCV trong sysroot cua Yocto SDK
find_package(OpenCV REQUIRED)

# Tim kiem thu vien ho tro da luong (Pthreads tren Linux)
find_package(Threads REQUIRED)

# In log ra man hinh de kiem tra duong dan thu vien co dung cua Yocto khong
message(STATUS "--- Kiem tra cau hinh build ---")
message(STATUS "Trinh bien dich: ${CMAKE_CXX_COMPILER}")
message(STATUS "Phien ban OpenCV: ${OpenCV_VERSION}")
message(STATUS "Duong dan OpenCV: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "--------------------------------")

# Tao file thuc thi 'dms_core_v2' tu file ma nguon 'dms_core_v2.cpp'
add_executable(dms_core_v2 src/dms_core_v2.cpp)

# Them cac co bien dich (Flags) de toi uu hieu suat cho Raspberry Pi
# -Wall -Wextra: Hien thi canh bao loi tiem an
# -O2: Toi uu hoa code de chay nhanh hon (Ruy tri FPS cao)
target_compile_options(dms_core_v2 PRIVATE -Wall -Wextra -O2)

# Chi dinh duong dan toi cac file Header cua OpenCV
target_include_directories(dms_core_v2 PRIVATE ${OpenCV_INCLUDE_DIRS})

# Lien ket (Link) cac thu vien OpenCV va Threads vao file thuc thi
target_link_libraries(dms_core_v2 
    PRIVATE 
    ${OpenCV_LIBS} 
    Threads::Threads
)
