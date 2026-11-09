# Harry Potter 3 (Prisoner of Azkaban) experimental sources.
# Included from ../flipendo.cmake. Disabled by default; HP3 uses UE2 and is not playable.
option(HP3_ENABLED "Enable experimental HP3/UE2 support (disabled by default)" OFF)

if(HP3_ENABLED)
    file(GLOB_RECURSE HP3_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/*.cpp
        ${CMAKE_CURRENT_LIST_DIR}/*.h)
    set(SURREALCOMMON_SOURCES ${SURREALCOMMON_SOURCES} ${HP3_SOURCES})
    include_directories(${CMAKE_CURRENT_LIST_DIR})
    source_group(TREE ${CMAKE_CURRENT_LIST_DIR} PREFIX "hp3" FILES ${HP3_SOURCES})
    add_compile_definitions(HP3_ENABLED=1)
    message(STATUS "HP3 experimental support ENABLED")
else()
    add_compile_definitions(HP3_ENABLED=0)
endif()
# Verified production helpers are guarded by IsHarryPotter3 at registration.
# They do not depend on the historical experimental HP3_ENABLED switch.
list(APPEND SURREALCOMMON_SOURCES ${CMAKE_CURRENT_LIST_DIR}/PoaCore.cpp)

list(APPEND SURREALCOMMON_SOURCES ${CMAKE_CURRENT_LIST_DIR}/PoaCompat.cpp)

# Production VM helpers include hp3 headers even when HP3_ENABLED is OFF.
include_directories(${CMAKE_CURRENT_LIST_DIR}/..)
list(APPEND SURREALCOMMON_SOURCES ${CMAKE_CURRENT_LIST_DIR}/PoaServices.cpp)

list(APPEND SURREALCOMMON_SOURCES ${CMAKE_CURRENT_LIST_DIR}/PoaActorAnimation.cpp)

list(APPEND SURREALCOMMON_SOURCES ${CMAKE_CURRENT_LIST_DIR}/PoaBones.cpp)

list(APPEND SURREALCOMMON_SOURCES ${CMAKE_CURRENT_LIST_DIR}/PoaNotifies.cpp)

list(APPEND SURREALCOMMON_SOURCES ${CMAKE_CURRENT_LIST_DIR}/PoaFloor.cpp)
