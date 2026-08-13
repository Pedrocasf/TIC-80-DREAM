################################
# TIC-80 app (PS Vita)
################################

# vita-elf-create cannot translate the GOT relocations -fPIC emits, and a couple
# of the vendored libraries (zip, pocketpy) turn it on for themselves
function(vita_disable_pic dir)
    get_property(targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target ${targets})
        get_target_property(type ${target} TYPE)
        if(NOT type STREQUAL "INTERFACE_LIBRARY" AND NOT type STREQUAL "UTILITY")
            set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
        endif()
    endforeach()

    get_property(subdirs DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirs})
        vita_disable_pic(${subdir})
    endforeach()
endfunction()

if(VITA)

    vita_disable_pic(${CMAKE_SOURCE_DIR})

    if(NOT DEFINED VITASDK)
        set(VITASDK $ENV{VITASDK})
    endif()

    # vita_create_self / vita_create_vpk
    include("${VITASDK}/share/vita.cmake" REQUIRED)

    set(VITA_APP_NAME "TIC-80")
    set(VITA_TITLEID  "TIC80VITA")

    # PARAM.SFO wants a strict "XX.YY" application version
    string(LENGTH "${VERSION_MAJOR}" VITA_VER_LEN)
    if(VITA_VER_LEN EQUAL 1)
        set(VITA_VER_MAJOR "0${VERSION_MAJOR}")
    else()
        set(VITA_VER_MAJOR "${VERSION_MAJOR}")
    endif()

    string(LENGTH "${VERSION_MINOR}" VITA_VER_LEN)
    if(VITA_VER_LEN EQUAL 1)
        set(VITA_VER_MINOR "0${VERSION_MINOR}")
    else()
        set(VITA_VER_MINOR "${VERSION_MINOR}")
    endif()

    set(VITA_VERSION "${VITA_VER_MAJOR}.${VITA_VER_MINOR}")

    # SDL's GXM renderer keeps its shaders in plain `unsigned char` arrays and
    # casts them to SceGxmProgram*, which gxm only accepts 4 byte aligned. GCC
    # pads data objects up to a word boundary only when it is not optimizing for
    # size, so -Os leaves every shader on an odd address and the renderer dies
    # with SCE_GXM_ERROR_INVALID_ALIGNMENT, i.e. a black screen. Build SDL at -O2
    # so the alignment upstream relies on is there in a MinSizeRel build too.
    if(TARGET SDL2-static)
        target_compile_options(SDL2-static PRIVATE -O2)
    endif()

    set(VITA_SCE_SYS ${CMAKE_SOURCE_DIR}/build/vita/sce_sys)

    target_sources(${TIC80_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/system/vita/runtime.c)

    target_link_libraries(${TIC80_TARGET}
        ScePower_stub
        SceAppMgr_stub
        SceAppUtil_stub
        SceSysmodule_stub
        SceIofilemgr_stub
        SceProcessmgr_stub
        m)

    # vita-elf-create needs the relocation info kept in the executable
    target_link_options(${TIC80_TARGET} PRIVATE -Wl,-q)

    # vita_create_self() looks the executable up next to the .self it builds,
    # so the ARM elf goes to the build root instead of bin/
    set_target_properties(${TIC80_TARGET} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    vita_create_self(${TIC80_TARGET}.self ${TIC80_TARGET} UNSAFE)

    vita_create_vpk(${TIC80_TARGET}.vpk ${VITA_TITLEID} ${TIC80_TARGET}.self
        VERSION ${VITA_VERSION}
        NAME    ${VITA_APP_NAME}
        FILE ${VITA_SCE_SYS}/icon0.png                      sce_sys/icon0.png
        FILE ${VITA_SCE_SYS}/livearea/contents/bg.png        sce_sys/livearea/contents/bg.png
        FILE ${VITA_SCE_SYS}/livearea/contents/startup.png   sce_sys/livearea/contents/startup.png
        FILE ${VITA_SCE_SYS}/livearea/contents/template.xml  sce_sys/livearea/contents/template.xml)

endif()
