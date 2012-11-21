#  OpenScienceMap binary vector tile format for OpenStreetMap

*.osmtile

## Header

SIZE - 4-bytes 32-bit integer

uint32_t size = h[0] << 24 | h[1] << 16 | h[2] << 8 | h[3];

## TAGS

Each element chunk is prefixed with TAG encoded as Varint32*

|-----------------------------
| TAG_TILE_NUM_TAGS = 1
|-----------------------------
| numTags : Varint32
|-----------------------------

|-----------------------------
| TAG_TILE_TAG_KEYS = 2
|-----------------------------
| tagIndex : 0..N Varint32
|-----------------------------

|-----------------------------
| TAG_TILE_TAG_VALUES = 3
|-----------------------------
|
| tileTags : 0...N String
|        (UTF-8)
|
|-----------------------------

|-----------------------------
| TAG_TILE_LINE = 11
|-----------------------------
| TAG_TILE_POLY = 12
|-----------------------------
| TAG_TILE_LINE = 13
|-----------------------------
|
|  TILE ELEMENT
|-----------------------------

## TILE ELEMENT

|-----------------------------
| TAG_ELEM_TAGS = 11
|-----------------------------
|
|  bytes : Varint32
|
|  0...N tagNum : Varint32
|
|
 ```
 if tagNum >= TAGS_MAX :
         tagNum -= TAGS_LIMIT # custom tags encoding
```
|-----------------------------


|-----------------------------
|
| TAG_ELEM_INDICES = 1
|-----------------------------
| indexCount : Varint32
|-----------------------------

|-----------------------------
| TAG_ELEM_INDEX = 12
|-----------------------------
|
| Array of Varint32
| size = indexCount
|
|-----------------------------

|-----------------------------
| TAG_ELEM_COORD = 13
|-----------------------------
|
| 0..N Varin32 X/Y pairs encoded
| using ZigZag algorithm
|
|-----------------------------

|-----------------------------
| TAG_ELEM_LAYER = 21
|-----------------------------
|
| ??
|-----------------------------


* - Varint32 encoding

** - ZigZag encoding
