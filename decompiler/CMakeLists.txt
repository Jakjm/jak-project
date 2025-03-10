add_library(
        decomp
        SHARED

        analysis/atomic_op_builder.cpp
        analysis/cfg_builder.cpp
        analysis/expression_build.cpp
        analysis/final_output.cpp
        analysis/find_defpartgroup.cpp
        analysis/find_defstates.cpp
        analysis/find_skelgroups.cpp
        analysis/inline_asm_rewrite.cpp
        analysis/insert_lets.cpp
        analysis/label_types.cpp
        analysis/mips2c.cpp
        analysis/reg_usage.cpp
        analysis/stack_spill.cpp
        analysis/static_refs.cpp
        analysis/symbol_def_map.cpp
        analysis/type_analysis.cpp
        analysis/variable_naming.cpp

        data/dir_tpages.cpp
        data/game_count.cpp
        data/game_text.cpp
        data/streamed_audio.cpp
        data/StrFileReader.cpp
        data/TextureDB.cpp
        data/tpage.cpp

        Disasm/Instruction.cpp
        Disasm/InstructionDecode.cpp
        Disasm/InstructionMatching.cpp
        Disasm/InstructionParser.cpp
        Disasm/OpcodeInfo.cpp
        Disasm/Register.cpp

        Function/BasicBlocks.cpp
        Function/CfgVtx.cpp
        Function/Function.cpp
        Function/TypeInspector.cpp

        IR/BasicOpBuilder.cpp
        IR/IR.cpp

        IR2/AtomicOp.cpp
        IR2/AtomicOpForm.cpp
        IR2/AtomicOpTypeAnalysis.cpp
        IR2/bitfields.cpp
        IR2/Env.cpp
        IR2/ExpressionHelpers.cpp
        IR2/Form.cpp
        IR2/FormExpressionAnalysis.cpp
        IR2/FormStack.cpp
        IR2/GenericElementMatcher.cpp
        IR2/LabelDB.cpp
        IR2/OpenGoalMapping.cpp

        level_extractor/extract_level.cpp
        level_extractor/extract_tfrag.cpp
        level_extractor/extract_tie.cpp
        level_extractor/BspHeader.cpp

        ObjectFile/LinkedObjectFile.cpp
        ObjectFile/LinkedObjectFileCreation.cpp
        ObjectFile/ObjectFileDB.cpp
        ObjectFile/ObjectFileDB_IR2.cpp

        util/config_parsers.cpp
        util/data_decompile.cpp
        util/DataParser.cpp
        util/DecompilerTypeSystem.cpp
        util/goal_data_reader.cpp
        util/sparticle_decompile.cpp
        util/TP_Type.cpp

        VuDisasm/VuDisassembler.cpp
        VuDisasm/VuInstruction.cpp

        config.cpp)

target_link_libraries(decomp
        lzokay
        common
        fmt
        )

add_executable(decompiler
        main.cpp)

target_link_libraries(decompiler
        decomp
        common
        lzokay
        fmt)
