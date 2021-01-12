{
    files = {
        "ekon.c"
    },
    values = {
        "g++",
        {
            "-m64",
            "-g",
            "-O0",
            "-std=c99"
        }
    },
    depfiles_gcc = "build/.objs/ekonpp/linux/x86_64/debug/ekon.c.o: ekon.c ekon.h common.h\
"
}