cmake_minimum_required(VERSION 3.10)
project(OpenVoice VERSION 1.0.0 LANGUAGES C)

# ---------------------------------------------------------------------------
# Enforce C99
# ---------------------------------------------------------------------------
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Compiler warnings
# ---------------------------------------------------------------------------
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -D_POSIX_C_SOURCE=200809L
)

# ---------------------------------------------------------------------------
# Find required libraries
# ---------------------------------------------------------------------------
find_package(Curses REQUIRED)

# ---------------------------------------------------------------------------
# Main application
# ---------------------------------------------------------------------------
set(SOURCES
    src/main.c
    src/ui.c
    src/speech.c
    src/phrase.c
    src/profile.c
    src/export.c
)

add_executable(openvoice ${SOURCES})

target_include_directories(openvoice PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CURSES_INCLUDE_DIR}
)

target_link_libraries(openvoice PRIVATE ${CURSES_LIBRARIES})

# ---------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------
install(TARGETS openvoice RUNTIME DESTINATION bin)
install(DIRECTORY data/ DESTINATION share/openvoice/data)

# ---------------------------------------------------------------------------
# Unit tests
# ---------------------------------------------------------------------------
enable_testing()

# test_profile
add_executable(test_profile
    tests/test_profile.c
    src/profile.c
)
target_include_directories(test_profile PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
add_test(NAME ProfileTests COMMAND test_profile)

# test_phrase
add_executable(test_phrase
    tests/test_phrase.c
    src/phrase.c
)
target_include_directories(test_phrase PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
add_test(NAME PhraseTests COMMAND test_phrase)

# test_speech
add_executable(test_speech
    tests/test_speech.c
    src/speech.c
    src/profile.c
)
target_include_directories(test_speech PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
add_test(NAME SpeechTests COMMAND test_speech)

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
message(STATUS "")
message(STATUS "  OpenVoice ${PROJECT_VERSION} configuration:")
message(STATUS "    C Standard : C${CMAKE_C_STANDARD}")
message(STATUS "    Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "    ncurses    : ${CURSES_LIBRARIES}")
message(STATUS "")
