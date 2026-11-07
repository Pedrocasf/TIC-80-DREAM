################################
# GXM shader alignment guard
################################
#
# SDL keeps its GXM shader programs in plain `unsigned char` arrays and casts
# them to SceGxmProgram*, so nothing in the source asks for their alignment. GCC
# pads data objects up to a word boundary only when it is not optimizing for
# size, and a misaligned one does not fail the build, it fails on the device:
# sceGxmProgramCheck() rejects the program with SCE_GXM_ERROR_INVALID_ALIGNMENT,
# creating the renderer fails, and TIC-80 runs on drawing into nothing. That is a
# black screen with no crash and no message, so check it here instead.

if(NOT NM OR NOT ELF)
    message(FATAL_ERROR "vita_check_alignment: NM and ELF must both be set")
endif()

execute_process(
    COMMAND ${NM} ${ELF}
    OUTPUT_VARIABLE SYMBOLS
    ERROR_VARIABLE NM_ERROR
    RESULT_VARIABLE NM_RESULT)

if(NOT NM_RESULT EQUAL 0)
    message(FATAL_ERROR "vita_check_alignment: ${NM} failed: ${NM_ERROR}")
endif()

string(REGEX MATCHALL "[0-9a-fA-F]+ [a-zA-Z] gxm_shader_[A-Za-z0-9_]+"
    SHADERS "${SYMBOLS}")

if(NOT SHADERS)
    # a stripped build is legitimate, say plainly that nothing was verified
    message(WARNING
        "vita_check_alignment: no gxm_shader_* symbols found in ${ELF}, "
        "shader alignment has NOT been verified")
    return()
endif()

set(MISALIGNED "")

foreach(SHADER IN LISTS SHADERS)
    string(REGEX MATCH "^[0-9a-fA-F]+" ADDRESS "${SHADER}")
    string(REGEX MATCH "gxm_shader_[A-Za-z0-9_]+$" NAME "${SHADER}")

    math(EXPR REMAINDER "0x${ADDRESS} % 4")

    if(NOT REMAINDER EQUAL 0)
        list(APPEND MISALIGNED "${NAME} at 0x${ADDRESS}")
    endif()
endforeach()

if(MISALIGNED)
    list(LENGTH MISALIGNED COUNT)
    string(REPLACE ";" "\n    " MISALIGNED "${MISALIGNED}")
    message(FATAL_ERROR
        "${COUNT} of SDL's GXM shader programs are not 4 byte aligned:\n"
        "    ${MISALIGNED}\n"
        "gxm rejects those with SCE_GXM_ERROR_INVALID_ALIGNMENT and the app "
        "boots to a black screen. Building SDL at -O2 is what keeps them "
        "aligned, check that cmake/vita.cmake still does so.")
endif()

list(LENGTH SHADERS COUNT)
message(STATUS "gxm shader alignment ok (${COUNT} programs)")
