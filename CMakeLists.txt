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
    includes/controllers/User.cpp
    includes/controllers/Auth.cpp
    includes/data/Connection.cpp
    includes/data/Master.cpp
    includes/data/PersonRepo.cpp
    includes/helpers/TokenManager.cpp
    includes/services/PersonService.cpp
)

# add_subdirectory(${THIRD_PARTY_DIR}/cppdb/lib)

include_directories(${INC_BOOSTER})
include_directories(${INC_CPPCMS})
include_directories(${CMAKE_SOURCE_DIR}/includes)
include_directories(${THIRD_PARTY_DIR}/cppdb/include)
include_directories(${THIRD_PARTY_DIR}/picojson)
link_directories(${THIRD_PARTY_DIR}/cppdb/lib)

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

target_link_libraries(${PROJECT_NAME}
    ${LIB_BOOSTER}
    ${LIB_CPPCMS}
    cppdb
)