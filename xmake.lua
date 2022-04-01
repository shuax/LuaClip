includes("VC-LTL5.lua")

set_policy("build.across_targets_in_parallel", false)

add_rules("mode.debug", "mode.release")

if is_mode("release") then
    add_defines("NDEBUG")
    add_cxflags("/Os", "/MT", "/EHsc", "/sdl-", "/GS-", "/GL", "/Gy")
    add_ldflags("/DYNAMICBASE", "/LTCG")
end

add_links("gdiplus", "kernel32", "user32", "gdi32", "winspool", "comdlg32")
add_links("advapi32", "shell32", "ole32", "oleaut32", "uuid", "odbc32", "odbccp32")
add_links("wininet", "shlwapi", "comctl32")

rule("dasc")
    set_extensions(".dasc")
    before_build_file(function(target, sourcefile)

        local outputdir = target:objectdir()
        if not os.isdir(outputdir) then
            os.mkdir(outputdir)
        end
        local argv = {"luajit/dynasm/dynasm.lua", "-LN"}
        table.insert(argv, "-D")
        table.insert(argv, "JIT")
        table.insert(argv, "-D")
        table.insert(argv, "FFI")
        if is_plat("windows", "mingw") then
            table.insert(argv, "-D")
            table.insert(argv, "WIN")
        end
        if is_arch("x64", "x86_64", "arm64", "arm64-v8a", "mips64") then
            table.insert(argv, "-D")
            table.insert(argv, "P64")
        end
        table.insert(argv, "-o")
        table.insert(argv, path.join(outputdir, "buildvm_arch.h"))
        table.insert(argv, sourcefile)
        os.vrunv(target:dep("minilua"):targetfile(), argv)
        target:add("includedirs", outputdir, {public = true})
    end)

rule("buildvm")
    before_build_files(function (target, sourcebatch)

        local buildvm = target:dep("buildvm")
        local outputdir = buildvm:objectdir()
        if not os.isdir(outputdir) then
            os.mkdir(outputdir)
        end

        local buildvm_bin = buildvm:targetfile()
        local sourcefiles = sourcebatch.sourcefiles
        os.vrunv(buildvm_bin, {"-m", "bcdef", "-o", "luajit/src/lj_bcdef.h", unpack(sourcefiles)})
        os.vrunv(buildvm_bin, {"-m", "ffdef", "-o", "luajit/src/lj_ffdef.h", unpack(sourcefiles)})
        os.vrunv(buildvm_bin, {"-m", "libdef", "-o", "luajit/src/lj_libdef.h", unpack(sourcefiles)})
        os.vrunv(buildvm_bin, {"-m", "recdef", "-o", "luajit/src/lj_recdef.h", unpack(sourcefiles)})
        os.vrunv(buildvm_bin, {"-m", "vmdef", "-o", "luajit/src/lj_vmdef.h", unpack(sourcefiles)})
        os.vrunv(buildvm_bin, {"-m", "folddef", "-o", "luajit/src/lj_folddef.h", "luajit/src/lj_opt_fold.c"})
        if is_plat("windows", "mingw") then
            local lj_vm_obj = path.join(outputdir, "lj_vm.obj")
            os.vrunv(buildvm_bin, {"-m", "peobj", "-o", lj_vm_obj})
            table.join2(target:objectfiles(), lj_vm_obj)
        else
            import("core.tool.compiler")
            local lj_vm_asm = path.join(outputdir, "lj_vm.S")
            local lj_vm_obj = path.join(outputdir, "lj_vm.o")
            local march
            if is_plat("macosx", "iphoneos", "watchos") then
                march = "machasm"
            else
                march = "elfasm"
            end
            os.vrunv(buildvm_bin, {"-m", march, "-o", lj_vm_asm})
            compiler.compile(lj_vm_asm, lj_vm_obj, {target = target})
            table.join2(target:objectfiles(), lj_vm_obj)
        end
    end)

target("minilua")
    set_kind("binary")
    add_files("luajit/src/host/minilua.c")

target("buildvm")
    set_kind("binary")
    add_deps("minilua")
    add_rules("dasc")
    add_includedirs("luajit/src", {public = true})
    add_files("luajit/src/host/buildvm*.c")
    if is_arch("x86", "i386") then
        add_files("luajit/src/vm_x86.dasc")
        add_defines("LUAJIT_TARGET=LUAJIT_ARCH_X86", {public = true})
    elseif is_arch("x64", "x86_64") then
        add_files("luajit/src/vm_x64.dasc")
        add_defines("LUAJIT_TARGET=LUAJIT_ARCH_X64", {public = true})
        -- add_defines("LUAJIT_ENABLE_GC64", {public = true})
    end

target("lua51")
    set_kind("static")
    add_deps("buildvm")
    add_defines("LUAJIT_UNWIND_EXTERNAL")
    add_defines("LUAJIT_ENABLE_LUA52COMPAT", {public = true})
    add_files("luajit/src/ljamalg.c")
    add_files("luajit/src/lib_base.c",
              "luajit/src/lib_math.c",
              "luajit/src/lib_bit.c",
              "luajit/src/lib_string.c",
              "luajit/src/lib_table.c",
              "luajit/src/lib_io.c",
              "luajit/src/lib_os.c",
              "luajit/src/lib_package.c",
              "luajit/src/lib_debug.c",
              "luajit/src/lib_jit.c",
              "luajit/src/lib_ffi.c",
              "luajit/src/lib_buffer.c", {rules = {"buildvm", override = true}})

target("LuaClip")
    set_kind("binary")
    set_targetdir("$(buildir)/release")
    add_deps("lua51")
    add_files("src/LuaClip.cpp")
    add_files("src/resource.rc")
    add_defines("UNICODE", "_UNICODE")
    after_build(function (target)
        os.cp("lua/*", "$(buildir)/release")
    end)
