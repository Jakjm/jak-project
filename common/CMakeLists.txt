add_library(common
        SHARED
        audio/audio_formats.cpp
        cross_os_debug/xdbg.cpp
        cross_sockets/xsocket.cpp
        custom_data/TFrag3Data.cpp
        dma/dma.cpp
        dma/dma_copy.cpp
        dma/gs.cpp

        goos/Interpreter.cpp
        goos/Object.cpp
        goos/ParseHelpers.cpp
        goos/Printer.cpp
        goos/PrettyPrinter.cpp
        goos/PrettyPrinter2.cpp
        goos/Reader.cpp
        goos/TextDB.cpp
        goos/ReplUtils.cpp
        math/geometry.cpp
        log/log.cpp
        type_system/defenum.cpp
        type_system/deftype.cpp
        type_system/state.cpp
        type_system/Type.cpp
        type_system/TypeFieldLookup.cpp
        type_system/TypeSpec.cpp
        type_system/TypeSystem.cpp
        util/Assert.cpp
        util/BitUtils.cpp
        util/compress.cpp
        util/dgo_util.cpp
        util/DgoReader.cpp
        util/DgoWriter.cpp
        util/diff.cpp
        util/FileUtil.cpp
        util/json_util.cpp
        util/Timer.cpp
        util/os.cpp
        util/print_float.cpp
        util/FontUtils.cpp
        util/FrameLimiter.cpp
        util/image_loading.cpp
        goos/Printer.cpp
        goos/PrettyPrinter2.cpp
        )

target_link_libraries(common fmt lzokay replxx libzstd_static)

if(WIN32)
    target_link_libraries(common wsock32 ws2_32 windowsapp)
else()
    target_link_libraries(common stdc++fs)
endif()
