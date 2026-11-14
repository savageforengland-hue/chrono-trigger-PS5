cmake_minimum_required(VERSION 3.25)
project(CTPS5 C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(EGG_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
set(EGG_GEN "" CACHE PATH "Generated C from the supported USA ROM")
set(SDL3_SOURCE "" CACHE PATH "OpenAGC SDL3 checkout")
set(SDL3_BUILD "" CACHE PATH "Built PS5 SDL3")
set(EGG_PLATFORM_MODULE "${CMAKE_CURRENT_LIST_DIR}/platform.c" CACHE FILEPATH "PS5 service/bootstrap module")
set(EGG_FEATURE_MODULE "${CMAKE_CURRENT_LIST_DIR}/optional_features.c" CACHE FILEPATH "Optional feature implementation")
option(EGG_PS5_SYSTEM_KEYBOARD "Load the optional native keyboard and IME modules" OFF)
set(HB "${PS5_PAYLOAD_SDK}/target/user/homebrew")
if(NOT EXISTS "${EGG_GEN}/dispatch.c")
    message(FATAL_ERROR "EGG_GEN must contain recompiled game sources")
endif()
file(GLOB RUNTIME_SOURCES "${EGG_ROOT}/runtime/*.c")
list(FILTER RUNTIME_SOURCES EXCLUDE REGEX "/game_build.c$")
list(FILTER RUNTIME_SOURCES EXCLUDE REGEX "/(cheevos|movie|music|shader|shader_glsl|translation)\\.c$")
set_source_files_properties(${RUNTIME_SOURCES} PROPERTIES COMPILE_DEFINITIONS "EGG_PS5_CORE=1")
file(GLOB GAME_SOURCES "${EGG_GEN}/*.c")
file(GLOB RC_SOURCES "${EGG_ROOT}/ps5/vendor/rcheevos/src/*.c"
    "${EGG_ROOT}/ps5/vendor/rcheevos/src/rapi/*.c"
    "${EGG_ROOT}/ps5/vendor/rcheevos/src/rcheevos/*.c"
    "${EGG_ROOT}/ps5/vendor/rcheevos/src/rhash/*.c")
list(FILTER RC_SOURCES EXCLUDE REGEX "/(rc_libretro|cdreader|hash_disc|hash_zip|hash_encrypted|aes)\\.c$")
add_library(egg_objects OBJECT ${RUNTIME_SOURCES} ${GAME_SOURCES}
    game_prepare.c diagnostics.c memory.c prompts.c presentation.c compat/directory.c compat/fp.c
    compat/heap.c compat/heap_wrap.c compat/placement.c compat/dlmalloc/mspace.c
    "${EGG_PLATFORM_MODULE}" config.c mods.c "${EGG_FEATURE_MODULE}")
if(NOT EGG_PS5_SYSTEM_KEYBOARD)
    target_sources(egg_objects PRIVATE controller_only.c)
endif()
set_source_files_properties(diagnostics.c PROPERTIES COMPILE_OPTIONS "-fno-builtin")
set_source_files_properties(memory.c compat/heap.c compat/heap_wrap.c compat/placement.c
    PROPERTIES INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/compat")
set_source_files_properties(compat/directory.c PROPERTIES COMPILE_DEFINITIONS "EGG_PS5_DIRECTORY_STREAMS_ONLY=1")
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/egg_objects.list"
    CONTENT "$<JOIN:$<TARGET_OBJECTS:egg_objects>,\n>\n")
target_include_directories(egg_objects PRIVATE "${EGG_ROOT}/runtime" "${EGG_GEN}"
    "${SDL3_SOURCE}/include" "${HB}/include" "${EGG_ROOT}/ps5/vendor/rcheevos/include")
target_compile_definitions(egg_objects PRIVATE EGG_PS5=1 EGG_STATIC_GAME=1 EGG_SINGLE_SCREEN=1
    EGG_VERSION="1.3.5-ps5-experimental" SDL_MAIN_HANDLED RC_CLIENT_SUPPORTS_HASH
    RC_HASH_NO_DISC RC_HASH_NO_ZIP RC_HASH_NO_ENCRYPTED)
target_compile_options(egg_objects PRIVATE -O1 -fwrapv -fPIC -ffunction-sections -fdata-sections)
add_executable(eggoftime_ps5 $<TARGET_OBJECTS:egg_objects>)
set_target_properties(eggoftime_ps5 PROPERTIES SUFFIX ".elf")
target_link_directories(eggoftime_ps5 PRIVATE "${HB}/lib")
target_link_options(eggoftime_ps5 PRIVATE -Wl,--gc-sections -Wl,-Map,eggoftime_ps5.map)
foreach(name malloc calloc realloc free posix_memalign aligned_alloc memalign malloc_usable_size reallocf reallocarray getline getdelim)
    target_link_options(eggoftime_ps5 PRIVATE "-Wl,--wrap=${name}")
endforeach()
target_link_libraries(eggoftime_ps5 PRIVATE "${SDL3_BUILD}/libSDL3.a"
    SceAudioOut ScePad SceSystemService SceUserService SceVideoOut)
if(EGG_PS5_SYSTEM_KEYBOARD)
    target_link_libraries(eggoftime_ps5 PRIVATE SceImeDialog SceKeyboard)
endif()
