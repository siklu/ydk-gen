cmake_minimum_required(VERSION 3.0.0)
cmake_policy(SET CMP0048 NEW)
project(ydk VERSION 0.8.4 LANGUAGES C CXX)

set(YDK_DESCRIPTION "YANG Development Kit Library. The library for  YDK API.")

# CPack Dynamic Settings
set (CPACK_PACKAGE_NAME "libydk")
set (CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set (CPACK_PACKAGE_RELEASE "1")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "YDK Core Library")
# set (CPACK_PACKAGE_DESCRIPTION_SUMMARY ${YDK_DESCRIPTION})

# CPack Static Settings
set (CPACK_PACKAGE_DESCRIPTION "An SDK that provides API's that are modeled in YANG.
The main goal of YDK is to reduce the learning curve of YANG data
models by expressing the model semantics in an API and abstracting
protocol/encoding details. YDK is composed of a core package that
defines services and providers, plus one or more module bundles that
are based on YANG models. Each module bundle is generated using a
bundle profile and the ydk-gen tool.")

set (CPACK_PACKAGE_CONTACT "ydk-admin@cisco.com")
set (CPACK_PACKAGE_VENDOR "Cisco")
set (CPACK_SET_DESTDIR true)

# MAC OS -- not working
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message("CMake System: MacOS")
    # set(CPACK_GENERATOR "productbuild")
    # set(CPACK_COMMAND_PKGBUILD "/usr/bin/pkgbuild")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_program(LSB_RELEASE lsb_release)
    execute_process(COMMAND ${LSB_RELEASE} -is
        OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Linux Debian
    if (${LSB_RELEASE_ID_SHORT} MATCHES "Ubuntu")
        message("CMake System: Ubuntu")
        set (CPACK_GENERATOR "DEB")

        set (CPACK_DEBIAN_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
        set (CPACK_DEBIAN_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
        set (CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
        set (CPACK_DEBIAN_PACKAGE_DEPENDS
            libxml2-dev,
            libtool-bin,
            cmake (>=3),
            git,
            pkg-config")
        set (CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_CONTACT})
        set (CPACK_DEBIAN_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
        set (CPACK_DEBIAN_PACKAGE_SECTION "devel")
        set (CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
        set (CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/CiscoDevNet/ydk-cpp")

        # <foo>_<VersionNumber>-<DebianRevisionNumber>_<DebianArchitecture>.deb
        # libydk_0.6.0-0.1.alpha_amd64.deb   // Debian
        set (CPACK_PACKAGE_FILE_NAME
            "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

    # Linux Fedora
    elseif(${LSB_RELEASE_ID_SHORT} MATCHES "CentOS")
        message("CMake System: CentOS")

        set (CPACK_GENERATOR "RPM")

        set (CPACK_RPM_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
        set (CPACK_RPM_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
        set (CPACK_RPM_PACKAGE_RELEASE ${CPACK_PACKAGE_RELEASE})
        set (CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
        set (CPACK_RPM_PACKAGE_SUMMARY ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
        set (CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION})

        set (CPACK_RPM_PACKAGE_LICENSE "Apache 2.0")
        set (CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
        set (CPACK_RPM_PACKAGE_VENDOR ${CPACK_PACKAGE_VENDOR})
        set (CPACK_RPM_PACKAGE_URL "https://github.com/CiscoDevNet/ydk-cpp")
        set (CPACK_RPM_PACKAGE_REQUIRES "python-pip, python-devel, libxml2-devel, libtool, cmake3)

        # <name>-<version>-<release>.<architecture>.(src).rpm
        # libydk-0.6.0-0.1.alpha.x86_64.rpm  // RPM
        set (CPACK_PACKAGE_FILE_NAME
            "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CPACK_RPM_PACKAGE_ARCHITECTURE}")
    endif()
endif()

include(CPack)

# include custom Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

if(NOT LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR lib)
endif()

if(NOT INCLUDE_INSTALL_DIR)
    set(INCLUDE_INSTALL_DIR include/ydk)
endif()

# set default build type if not specified by user
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE debug)
endif()

set(HOME_DIRECTORY $ENV{HOME})

set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

if(COVERAGE STREQUAL True)
    set(CMAKE_CXX_FLAGS_DEBUG   "-g3 -O0 -fprofile-arcs -ftest-coverage")
    set(CMAKE_BUILD_TYPE debug)
else()
    set(CMAKE_CXX_FLAGS_DEBUG   "-g3 -O0")
endif()

if(CMAKE_BUILD_TYPE STREQUAL debug)
    option(ENABLE_BUILD_TESTS "Build tests" ON)
    option(ENABLE_VALGRIND_TESTS "Build tests with valgrind" ON)
else()
    option(ENABLE_BUILD_TESTS "Build tests" OFF)
    option(ENABLE_VALGRIND_TESTS "Build tests with valgrind" OFF)
endif()

# Set LibYDK variables
set(libydk_src
    src/bits.cpp
    src/entity.cpp
    src/entity_util.cpp
    src/value.cpp
    src/value_list.cpp
    src/xml_subtree_codec.cpp
    src/leaf_data.cpp
    src/xml_util.cpp
    src/ydk_operation.cpp
    )

set(libydk_install_headers
    src/entity_util.hpp
    src/filters.hpp
    src/logger.hpp
    src/types.hpp
    src/xml_subtree_codec.hpp
    src/xml_util.hpp
    src/types.hpp)

# Find dependent libraries
find_library(pthread_location pthread)
find_library(dl_location dl)
find_library(m_location m)
find_package(LibXml2 REQUIRED)


include_directories(
    ${LIBXML2_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(ydk STATIC ${libydk_src})

target_link_libraries(ydk
    ${LIBXML2_LIBRARIES}
    ${pthread_location}
    -rdynamic
    ${dl_location}
)

set_property(TARGET ydk PROPERTY CXX_STANDARD 11)
set_property(TARGET ydk PROPERTY CXX_STANDARD_REQUIRED ON)

install(TARGETS ydk DESTINATION ${LIB_INSTALL_DIR})
install(FILES ${libydk_install_headers} DESTINATION ${INCLUDE_INSTALL_DIR})

# generate doxygen documentation for ydk_core API
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_SKIP_DOT TRUE)
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    configure_file(Doxyfile.in Doxyfile)
endif()

# clean cmake cache
add_custom_target(cclean
    COMMAND make clean
    COMMAND find . -iname '*cmake*' -not -name CMakeLists.txt -not -path './CMakeModules*' -exec rm -rf {} +
    COMMAND rm -rf Makefile Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(ENABLE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif(ENABLE_BUILD_TESTS)
