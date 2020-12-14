import json
import hashes
import strutils
import lexbase
import macros
import options
import streams
import tables


type
  EkonType = enum
    EKON_TYPE_BOOL, EKON_TYPE_ARRAY, EKON_TYPE_OBJECT, EKON_TYPE_STRING,
    EKON_TYPE_NULL, EKON_TYPE_NUMBER
  CEkonNode = object {.importc:"EkonNode",header:"../ekon.h".}
    ekonType: EkonType 
    key: cstring
    keyLen: cuint
    isKeySpaced: bool
    len: cuint
    isStrSpaced: bool
    isStrMultilined: bool
    next: ref EkonNode
    prev: ref EkonNode
    father: ref EkonNode
    value: EkonVal  
  EkonVal = object {.union.}
    node: ref EkonNode
    str: cstring

type EkonAllocator = object {.importc:"EkonAllocator",header:"../ekon.h".}

type EkonValue = object
  n: ref EkonNode
  a: ref EkonAllocator


# ---- Public API Types ----
type EkonNodeKind* = json.JsonNodeKind
type EkonNodeObj* = json.JsonNodeObj
type EkonNode* = json.JsonNode
# --------------------------

proc ekonValueParseFast(v: ref EkonValue, s: cstring, err: ref cstring): bool {.importc: "ekonValueParseFast", header: "../ekon.h"}
proc ekonValueStringify(v: ref EkonValue): cstring {.importc:"ekonValueStringify",header:"../ekon.h"}
proc ekonValueStringifyToJSON(v: ref EkonValue): cstring {.importc: "ekonValueStringifyToJSON", header: "../ekon.h"}

proc convertEkonNodeToJsonNode(v: ref EkonNode): JsonNode
proc convertJsonNodeToEkonNode(v: ref JsonNode): EkonNode

# ---- Public API Procs ----
proc parseEkon*(s: Stream, filename: string = ""): EkonNode
proc parseEkon*(buffer: string): EkonNode
proc parseEkon*(buffer: cstring): EkonNode

proc toUgly*(result: var string, node: EkonNode)
proc pretty(node: EkonNode, index = 2): string

proc newEString*(s: string): EkonNode
proc newEInt*(n: BiggestInt): EkonNode
proc newEFloat*(n: float): EkonNode
proc newEBool*(b: bool): EkonNode
proc newENull(): EkonNode
proc newEObject(): EkonNode
proc newEArray(): EkonNode

proc getStr*(n: EkonNode, default: string = ""): string
proc getInt*(n: EkonNode, default: int = 0): int
proc getBiggestInt*(n: EkonNode, default: int = 0): int
proc getFloat*(n: EkonNode, default: float = 0.0): float
proc getBool*(n: EkonNode, default: bool = false): bool
proc getFields*(n: EkonNode, default = initOrderedTable(2)): OrderedTable[string, EkonNode]
proc getElems*(n: EkonNode, default: seq[EkonNode] = @[]): seq[EkonNode]
proc add*(father: EkonNode, child: EkonNode)
proc add*(obj: EkonNode, key: string, val: EkonNode)
proc `%`*(s: string): EkonNode
proc `%`*(n: uint): EkonNode
proc `%`*(n: int): EkonNode
proc `%`*(n: BiggestUInt): EkonNode
proc `%`*(n: BiggestInt): EkonNode
proc `%`*(n: float): EkonNode
proc `%`*(n: bool): EkonNode
proc `%`*(keyVals: openArray[tuple[key: string, val: EkonNode]]): EkonNode
proc `%`*[T](elements: openArray[T]): EkonNode
proc `%`*[T](table: Table[string, T] | OrderedTable[string, T]): EkonNode
proc `%`*[T](opt: Option[T])
proc `%`*[T: object](o: T): EkonNode
proc `%`*(o: ref object): EkonNode
proc `%`*(o: enum): EkonNode

proc `==`*(a, b: EkonNode): bool



# --------------------------


