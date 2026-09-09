cmake_minimum_required(VERSION 3.25)

set(PROJECT_NAME cppcms_simple)
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

project(${PROJECT_NAME})
add_definitions(-std=c++11)

find_library(LIB_BOOSTER booster)
find_library(LIB_CPPCMS cppcms)

# set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")

find_path(INC_BOOSTER booster/config.h)
find_path(INC_CPPCMS cppcms/config.h)

set(API_SRC
    main.cpp
    src/controllers/User.cpp
    src/controllers/Auth.cpp
    src/controllers/Person.cpp
    src/data/Connection.cpp
    src/controllers/Master.cpp
    src/data/PersonRepo.cpp
    src/data/UserRepo.cpp
    src/helpers/TokenManager.cpp
    src/services/PersonService.cpp
)

# add_subdirectory(${THIRD_PARTY_DIR}/cppdb/lib)

include_directories(${INC_BOOSTER})
include_directories(${INC_CPPCMS})
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${THIRD_PARTY_DIR}/cppdb/include)
include_directories(${THIRD_PARTY_DIR}/picojson)

# Link cppdb by full path so CMake does not inject an absolute -rpath from link_directories().
if(EXISTS "${THIRD_PARTY_DIR}/cppdb/lib/libcppdb.so")
    set(CPPDB_SHARED_LIB "${THIRD_PARTY_DIR}/cppdb/lib/libcppdb.so")
elseif(EXISTS "${THIRD_PARTY_DIR}/cppdb/lib/libcppdb.dylib")
    set(CPPDB_SHARED_LIB "${THIRD_PARTY_DIR}/cppdb/lib/libcppdb.dylib")
else()
    message(FATAL_ERROR "Bundled cppdb shared library not found under ${THIRD_PARTY_DIR}/cppdb/lib")
endif()
add_library(cppdb SHARED IMPORTED)
set_target_properties(cppdb PROPERTIES IMPORTED_LOCATION "${CPPDB_SHARED_LIB}")

add_executable(${PROJECT_NAME} ${API_SRC})

# Set RPATH for macOS to find libraries relative to executable
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "${THIRD_PARTY_DIR}/cppdb/lib"
        INSTALL_RPATH "@executable_path/../third_party/cppdb/lib"
        MACOSX_RPATH TRUE
    )
    
    # Add post-build command to fix library paths for bundled libraries
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Fixing library paths..."
        COMMAND install_name_tool -change 
            /Users/pulung/Downloads/cppcms-code-r2420-cppdb-trunk/build/libcppdb.0.dylib
            ${THIRD_PARTY_DIR}/cppdb/lib/libcppdb.0.dylib
            $<TARGET_FILE:${PROJECT_NAME}>
        COMMAND install_name_tool -change 
            /Users/pulung/Downloads/cppcms-code-r2420-cppdb-trunk/build/libcppdb_sqlite3.0.dylib
            ${THIRD_PARTY_DIR}/cppdb/lib/libcppdb_sqlite3.0.dylib
            $<TARGET_FILE:${PROJECT_NAME}>
        COMMAND install_name_tool -change 
            /Users/pulung/Downloads/cppcms-code-r2420-cppdb-trunk/build/libcppdb_mysql.0.dylib
            ${THIRD_PARTY_DIR}/cppdb/lib/libcppdb_mysql.0.dylib
            $<TARGET_FILE:${PROJECT_NAME}>
        COMMENT "Updating library paths to use bundled libraries"
    )
endif()

if(UNIX AND NOT APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "\$ORIGIN/lib"
        INSTALL_RPATH "\$ORIGIN/lib"
        BUILD_RPATH_USE_ORIGIN TRUE
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH_USE_LINK_PATH FALSE
    )
endif()

target_link_libraries(${PROJECT_NAME}
    ${LIB_BOOSTER}
    ${LIB_CPPCMS}
    cppdb
)

enable_testing()
find_package(Threads REQUIRED)

set(PERSON_TEST_SRC
    tests/person_tests_main.cpp
    tests/person_controller_test.cpp
    src/controllers/Person.cpp
    src/controllers/Master.cpp
    src/data/PersonRepo.cpp
    src/helpers/TokenManager.cpp
    src/services/PersonService.cpp
)

add_executable(person_tests ${PERSON_TEST_SRC})
target_link_libraries(person_tests
    ${LIB_BOOSTER}
    ${LIB_CPPCMS}
    cppdb
    Threads::Threads
)
target_compile_definitions(person_tests PRIVATE
    "TEST_DB_PATH=\"${CMAKE_SOURCE_DIR}/test.db\""
)
set_target_properties(person_tests PROPERTIES
    BUILD_RPATH "${THIRD_PARTY_DIR}/cppdb/lib"
)
add_test(NAME person_tests COMMAND person_tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
install(FILES
    "${CMAKE_SOURCE_DIR}/config.json"
    "${CMAKE_SOURCE_DIR}/compose.yaml"
    "${CMAKE_SOURCE_DIR}/Dockerfile"
    "${CMAKE_SOURCE_DIR}/BUNDLE.md"
    DESTINATION .
)
install(FILES "${CMAKE_SOURCE_DIR}/db.db" DESTINATION . RENAME db.db.example)

if(UNIX AND NOT APPLE)
    # install(FILES) in this CMake does not accept FOLLOW_SYMLINK_CHAIN;
    # file(COPY) does, and copies the real .so plus SONAME links.
    function(install_so_chain lib dest)
        get_filename_component(_abs "${lib}" ABSOLUTE)
        install(CODE "
            set(_dest \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${dest}\")
            file(MAKE_DIRECTORY \"\${_dest}\")
            file(COPY \"${_abs}\" DESTINATION \"\${_dest}\" FOLLOW_SYMLINK_CHAIN)
        ")
    endfunction()
    install_so_chain("${LIB_BOOSTER}" lib)
    install_so_chain("${LIB_CPPCMS}" lib)
    install_so_chain("${CPPDB_SHARED_LIB}" lib)
endif()

set(BUNDLE_STAGE "${CMAKE_BINARY_DIR}/bundle")
set(BUNDLE_TARBALL "${CMAKE_BINARY_DIR}/cppcms-simple-bundle.tar.gz")
add_custom_target(bundle
    DEPENDS ${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${BUNDLE_STAGE}"
    COMMAND ${CMAKE_COMMAND} --install "${CMAKE_BINARY_DIR}" --prefix "${BUNDLE_STAGE}"
    COMMAND ${CMAKE_COMMAND} -E chdir "${BUNDLE_STAGE}"
            ${CMAKE_COMMAND} -E tar czf "${BUNDLE_TARBALL}" .
    BYPRODUCTS "${BUNDLE_TARBALL}"
    COMMENT "Installing runtime tree into ${BUNDLE_STAGE} and packing ${BUNDLE_TARBALL}"
    VERBATIM
)
