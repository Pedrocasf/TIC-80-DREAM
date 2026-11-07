################################
# LiveArea asset guard
################################
#
# The Vita installer refuses a package whose sce_sys images are not 8 bit
# indexed PNGs, it stops at 99% with 0x8010113D. Being indexed is not enough
# either, the bit depth has to be exactly 8, and an encoder handed artwork with
# four colors in it will happily pick 1, 2 or 4 instead. None of that is visible
# in an image viewer, so read the PNG headers here rather than find out on the
# device.

if(NOT SCE_SYS)
    message(FATAL_ERROR "vita_check_assets: SCE_SYS must be set")
endif()

# path below sce_sys, required width, required height
set(ASSETS
    "icon0.png|128|128"
    "livearea/contents/bg.png|840|500"
    "livearea/contents/startup.png|280|158")

foreach(ASSET IN LISTS ASSETS)
    string(REPLACE "|" ";" FIELDS "${ASSET}")
    list(GET FIELDS 0 NAME)
    list(GET FIELDS 1 WANT_WIDTH)
    list(GET FIELDS 2 WANT_HEIGHT)

    set(ASSET_PATH "${SCE_SYS}/${NAME}")

    if(NOT EXISTS "${ASSET_PATH}")
        message(FATAL_ERROR "vita_check_assets: ${ASSET_PATH} is missing")
    endif()

    # every PNG carries its signature and IHDR in the first 29 bytes
    file(READ "${ASSET_PATH}" HEADER HEX LIMIT 29)
    string(LENGTH "${HEADER}" HEADER_LENGTH)

    if(HEADER_LENGTH LESS 58)
        message(FATAL_ERROR "vita_check_assets: ${NAME} is truncated")
    endif()

    string(SUBSTRING "${HEADER}" 0 16 SIGNATURE)

    if(NOT SIGNATURE STREQUAL "89504e470d0a1a0a")
        message(FATAL_ERROR "vita_check_assets: ${NAME} is not a PNG")
    endif()

    string(SUBSTRING "${HEADER}" 32 8 WIDTH)
    string(SUBSTRING "${HEADER}" 40 8 HEIGHT)
    string(SUBSTRING "${HEADER}" 48 2 DEPTH)
    string(SUBSTRING "${HEADER}" 50 2 COLOR_TYPE)
    string(SUBSTRING "${HEADER}" 56 2 INTERLACE)

    math(EXPR WIDTH "0x${WIDTH}")
    math(EXPR HEIGHT "0x${HEIGHT}")
    math(EXPR DEPTH "0x${DEPTH}")
    math(EXPR COLOR_TYPE "0x${COLOR_TYPE}")
    math(EXPR INTERLACE "0x${INTERLACE}")

    # color type 3 means every pixel is an index into PLTE
    if(NOT DEPTH EQUAL 8 OR NOT COLOR_TYPE EQUAL 3)
        message(FATAL_ERROR
            "${NAME} has bit depth ${DEPTH} and color type ${COLOR_TYPE}, the "
            "Vita installer only takes 8 bit indexed PNGs (depth 8, color type "
            "3) under sce_sys and rejects the package with 0x8010113D "
            "otherwise. `pngquant --force 256 ${NAME}` produces one.")
    endif()

    if(NOT INTERLACE EQUAL 0)
        message(FATAL_ERROR "${NAME} must not be interlaced")
    endif()

    if(NOT WIDTH EQUAL WANT_WIDTH OR NOT HEIGHT EQUAL WANT_HEIGHT)
        message(FATAL_ERROR
            "${NAME} must be ${WANT_WIDTH}x${WANT_HEIGHT}, it is ${WIDTH}x${HEIGHT}")
    endif()
endforeach()

message(STATUS "LiveArea assets ok (8 bit indexed)")
