cmake_minimum_required(VERSION 3.16)

# ─────────────────────────────────────────────────────────
#  Project metadata
# ─────────────────────────────────────────────────────────
project(ExpenseTracker
    VERSION      1.0.0
    DESCRIPTION  "Multi-user expense tracker with CSV persistence and coloured terminal UI"
    LANGUAGES    CXX
)

# ─────────────────────────────────────────────────────────
#  C++ standard
# ─────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# ─────────────────────────────────────────────────────────
#  Build type default
# ─────────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Build type (Debug | Release | RelWithDebInfo | MinSizeRel)" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
        STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ─────────────────────────────────────────────────────────
#  Compiler warnings
# ─────────────────────────────────────────────────────────
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wshadow -Wconversion -Wsign-conversion
        $<$<CONFIG:Release>:-O2>
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g3 -O0 -fsanitize=address,undefined)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

# ─────────────────────────────────────────────────────────
#  Source target: core library (reusable in tests)
# ─────────────────────────────────────────────────────────
add_library(expense_tracker_lib INTERFACE)
target_include_directories(expense_tracker_lib
    INTERFACE
        ${PROJECT_SOURCE_DIR}/include
)

# ─────────────────────────────────────────────────────────
#  Main executable
# ─────────────────────────────────────────────────────────
add_executable(expense_tracker
    src/expense_tracker.cpp
)
target_link_libraries(expense_tracker PRIVATE expense_tracker_lib)

# Ensure the binary runs from the project root so CSV files
# (expenses.csv / users.csv) are found relative to that directory.
set_target_properties(expense_tracker PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
)

# ─────────────────────────────────────────────────────────
#  Copy seed data files next to the executable at build time
# ─────────────────────────────────────────────────────────
add_custom_command(TARGET expense_tracker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/data/expenses.csv"
        "$<TARGET_FILE_DIR:expense_tracker>/expenses.csv"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/data/users.csv"
        "$<TARGET_FILE_DIR:expense_tracker>/users.csv"
    COMMENT "Copying seed CSV data files to build output directory"
)

# ─────────────────────────────────────────────────────────
#  Tests (CTest + lightweight hand-rolled runner)
# ─────────────────────────────────────────────────────────
option(BUILD_TESTS "Build the test suite" ON)

if(BUILD_TESTS)
    enable_testing()
endif()

# ─────────────────────────────────────────────────────────
#  Install rules
# ─────────────────────────────────────────────────────────
include(GNUInstallDirs)

install(TARGETS expense_tracker
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES
    "${PROJECT_SOURCE_DIR}/data/expenses.csv"
    "${PROJECT_SOURCE_DIR}/data/users.csv"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/expense_tracker
)
install(FILES "${PROJECT_SOURCE_DIR}/docs/README.md"
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    OPTIONAL
)


# ─────────────────────────────────────────────────────────
#  Summary
# ─────────────────────────────────────────────────────────
message(STATUS "")
message(STATUS "=== Expense Tracker ${PROJECT_VERSION} ===")
message(STATUS "  Compiler : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Standard : C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Build    : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Tests    : ${BUILD_TESTS}")
message(STATUS "  Install  : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
