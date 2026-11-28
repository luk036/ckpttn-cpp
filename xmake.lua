add_rules("mode.debug", "mode.release", "mode.coverage")
add_requires("fmt", "spdlog", "nlohmann_json", "doctest", "nanobench")
add_requires("microsoft-gsl", {alias = "ms-gsl"})
add_requires("cxxopts", {system = false})

set_languages("c++20")
add_includedirs("include", "../py2cpp/include", "../xnetwork-cpp/include",
                "../mywheel-cpp/include", "../netlistx-cpp/include")

if is_plat("linux") then
    set_rundir("./build/linux/")
elseif is_plat("windows") then
    set_rundir("./build/windows/")
end

local toolchain = get_config("toolchain")
local is_msvc_like = (toolchain == nil) or (toolchain == "msvc") or (toolchain == "clang-cl") or (toolchain == "clang_cl")

if is_plat("linux") then
    set_warnings("all", "error")
    add_cxflags("-Wconversion", {force = true})
    -- Termux/Android ships its libc++ headers under $PREFIX
    local prefix = os.getenv("PREFIX")
    if prefix then
        add_cxflags("-Wno-unused-command-line-argument", {force = true})
        add_sysincludedirs(prefix .. "/include/c++/v1", prefix .. "/include", {public = true})
    end
elseif is_plat("windows") and is_msvc_like then
    add_cxflags("/EHsc /W4 /WX /openmp /wd4702 /wd4996", {force = true})
elseif is_plat("windows") then
    add_cxflags("-Wall", "-Wextra", {force = true})
end

if is_mode("coverage") then
    add_cxflags("-ftest-coverage", "-fprofile-arcs", {force = true})
end

if is_mode("profile") then
    set_optimize("fastest")
    set_symbols("debug")
    if not is_msvc_like then
        add_cxflags("-pg", "-fno-omit-frame-pointer", {force = true})
        add_ldflags("-pg", {force = true})
    end
end

target("CkPttn")
    set_kind("static")
    add_files("source/*.cpp", "source/midlevel/*.cpp",
              "../netlistx-cpp/source/netlist_algo.cpp",
              "../netlistx-cpp/source/readwrite.cpp",
              "../netlistx-cpp/source/reader.cpp")
    add_packages("ms-gsl", "spdlog", "nlohmann_json")

target("test_ckpttn")
    add_deps("CkPttn")
    add_files("test/source/*.cpp")
    add_packages("fmt", "doctest", "spdlog", "ms-gsl")
    add_tests("default")

target("ckpttn")
    add_deps("CkPttn")
    add_files("standalone/source/*.cpp")
    add_packages("cxxopts", "ms-gsl", "spdlog", "nlohmann_json")

-- bench_<target> = bench/<file>, same deps; nanobench only where used
local benches = {
    {"bench_fmbi_2pin_nets", "FMBi_2pin_nets.cpp", true},
    {"bench_fmbi_p1", "FMBi_p1.cpp", true},
    {"bench_fmkway_2pin_nets", "FMKWay_2pin_nets.cpp", true},
    {"bench_fmnn", "BenchFMNN.cpp", false},
    {"bench_ibm01", "BM_ibm01.cpp", true},
    {"bench_param_sweep", "ParamSweep.cpp", false},
    {"bench_minhash_probe", "MinhashProbe.cpp", false},
}
for _, b in ipairs(benches) do
    target(b[1])
        add_deps("CkPttn")
        add_files("bench/" .. b[2])
        add_packages("ms-gsl", "spdlog", "nlohmann_json")
        if b[3] then
            add_packages("nanobench")
        end
end
