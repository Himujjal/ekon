{
    depfiles_gcc = "build/.objs/tests/linux/x86_64/debug/tests/conformance/conformance_test.cpp.o:  tests/conformance/conformance_test.cpp ekon.h common.h tests/test.h\
",
    files = {
        "tests/conformance/conformance_test.cpp"
    },
    values = {
        "g++",
        {
            "-m64",
            "-g",
            "-O0",
            "-std=c++11",
            "-I.",
            "-Itests"
        }
    }
}