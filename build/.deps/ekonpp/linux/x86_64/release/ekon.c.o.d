{
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fvisibility=hidden",
            "-O3",
            "-std=c99"
        }
    },
    depfiles_gcc = "build/.objs/ekonpp/linux/x86_64/release/ekon.c.o: ekon.c ekon.h common.h  hashmap.h\
",
    files = {
        "ekon.c"
    }
}