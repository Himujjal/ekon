from os import fileExists, expandFilename

const libName* = "libmagic.so"

const  MAGIC_NONE              * = 0x0000000 # No flags 
const  MAGIC_DEBUG             * = 0x0000001 # Turn on debugging 
const  MAGIC_SYMLINK           * = 0x0000002 # Follow symlinks 
const  MAGIC_COMPRESS          * = 0x0000004 # Check inside compressed files 
const  MAGIC_DEVICES           * = 0x0000008 # Look at the contents of devices 
const  MAGIC_MIME_TYPE         * = 0x0000010 # Return the MIME type 
const  MAGIC_CONTINUE          * = 0x0000020 # Return all matches 
const  MAGIC_CHECK             * = 0x0000040 # Print warnings to stderr 
const  MAGIC_PRESERVE_ATIME    * = 0x0000080 # Restore access time on exit 
const  MAGIC_RAW               * = 0x0000100 # Don't convert unprintable chars 
const  MAGIC_ERROR             * = 0x0000200 # Handle ENOENT etc as real errors 
const  MAGIC_MIME_ENCODING     * = 0x0000400 # Return the MIME encoding 
const  MAGIC_APPLE             * = 0x0000800 # Return the Apple creator/type 
const  MAGIC_EXTENSION         * = 0x1000000 # Return a /-separated list of
const  MAGIC_COMPRESS_TRANSP   * = 0x2000000 # Check inside compressed files

const  MAGIC_NO_CHECK_COMPRESS * = 0x0001000 # Don't check for compressed files 
const  MAGIC_NO_CHECK_TAR      * = 0x0002000 # Don't check for tar files 
const  MAGIC_NO_CHECK_SOFT     * = 0x0004000 # Don't check magic entries 
const  MAGIC_NO_CHECK_APPTYPE  * = 0x0008000 # Don't check application type 
const  MAGIC_NO_CHECK_ELF      * = 0x0010000 # Don't check for elf details 
const  MAGIC_NO_CHECK_TEXT     * = 0x0020000 # Don't check for text files 
const  MAGIC_NO_CHECK_CDF      * = 0x0040000 # Don't check for cdf files 
const  MAGIC_NO_CHECK_CSV      * = 0x0080000 # Don't check for CSV files 
const  MAGIC_NO_CHECK_TOKENS   * = 0x0100000 # Don't check tokens 
const  MAGIC_NO_CHECK_ENCODING * = 0x0200000 # Don't check text encodings 
const  MAGIC_NO_CHECK_JSON     * = 0x0400000 # Don't check for JSON files 

type Magic  = object
type MagicPtr* = ptr Magic

# import the c header file & also make sure to link the dynamic library: libmagic.so
proc magic_open(i: cint): MagicPtr {.importc, dynlib:libName.}

proc magic_close(p: MagicPtr): void {.importc, dynlib:libName.}

proc magic_load(p: MagicPtr, s:cstring): cint {.importc, dynlib:libName.}

