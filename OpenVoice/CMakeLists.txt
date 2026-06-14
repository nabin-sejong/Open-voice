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
add_compile_options(-Wall -Wextra -Wpedantic)

# ---------------------------------------------------------------------------
# Main application  (Windows Console API — no external libraries needed)
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
)

# ---------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------
install(TARGETS openvoice RUNTIME DESTINATION bin)
install(DIRECTORY data/ DESTINATION share/openvoice/data)

# ---------------------------------------------------------------------------
# Unit tests  (no UI dependency — compile cleanly without windows.h tricks)
# ---------------------------------------------------------------------------
enable_testing()

add_executable(test_profile
    tests/test_profile.c
    src/profile.c
)
target_include_directories(test_profile PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
add_test(NAME ProfileTests COMMAND test_profile)

add_executable(test_phrase
    tests/test_phrase.c
    src/phrase.c
)
target_include_directories(test_phrase PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
add_test(NAME PhraseTests COMMAND test_phrase)

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
message(STATUS "    TTS engine : Windows SAPI (built-in)")
message(STATUS "")
