{
    depfiles_gcc = "build/.objs/tests/linux/x86_64/release/tests/conformance/conformance_test.cpp.o:  tests/conformance/conformance_test.cpp ekon.h common.h tests/test.h\
",
    files = {
        "tests/conformance/conformance_test.cpp"
    },
    values = {
        "g++",
        {
            "-m64",
            "-fvisibility=hidden",
            "-fvisibility-inlines-hidden",
            "-O3",
            "-std=c++11",
            "-I.",
            "-Itests"
        }
    }
}
