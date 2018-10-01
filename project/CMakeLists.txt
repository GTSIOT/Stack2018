cmake_minimum_required (VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
project (opensplice_minimal)

include (MacroOpenSplice)
include (osmacros)

include(CheckCXXCompilerFlag)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    check_cxx_compiler_flag(--std=c++11 SUPPORTS_CXX11)
    if(SUPPORTS_CXX11)
        add_compile_options(--std=c++11)
    else()
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

find_package (OpenSplice REQUIRED)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${OpenSplice_INCLUDE_DIRS}
  include
  gen
)

file(GLOB SERIAL_SOURCE
)


ADD_LIBRARY(GEN_SRC 
  gen/EnvironmentalData.cpp
  gen/EnvironmentalDataDcps.cpp
  gen/EnvironmentalDataDcps_impl.cpp
  gen/EnvironmentalDataSplDcps.cpp
)

TARGET_LINK_LIBRARIES (GEN_SRC 
 ${OpenSplice_LIBRARIES}
)

ADD_LIBRARY (MGR_SRC
    src/DDSEntityManager.cpp 
    src/CheckStatus.cpp
)

TARGET_LINK_LIBRARIES (MGR_SRC
 ${OpenSplice_LIBRARIES}
)


ADD_EXECUTABLE (edge_fake
    src/EnvironmentalDataPublisherFake.cpp
)

TARGET_LINK_LIBRARIES (edge_fake
    GEN_SRC
    MGR_SRC
    ${OpenSplice_LIBRARIES}
 )

 ADD_EXECUTABLE (c2 
    src/EnvironmentalDataSubscriber.cpp
)

TARGET_LINK_LIBRARIES (c2
    GEN_SRC
    MGR_SRC
    ${OpenSplice_LIBRARIES}
 )
