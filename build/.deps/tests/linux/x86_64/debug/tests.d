{
    files = {
        "build/.objs/tests/linux/x86_64/debug/tests/conformance/conformance_test.cpp.o",
        "build/linux/x86_64/debug/libekonpp.a"
    },
    values = {
        "g++",
        {
            "-m64",
            "-Lbuild/linux/x86_64/debug",
            "-lekonpp"
        }
    }
}