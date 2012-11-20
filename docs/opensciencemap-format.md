#  OpenScienceMap binary vector format for OpenStreetMap

*.osmtile

## Header

SIZE - 4-bytes 32-bit integer

Each element prefixed with Tag encoded as Varint32*

## TAGS


### TAG_TILE_NUM_TAGS = 1

numTags : Varint32

### TAG_TILE_TAG_KEYS = 2

tagIndex : ShortArray

### TAG_TILE_TAG_VALUES = 3

tileTags : String (UTF-8)

###  TAG_TILE_LINE = 11
###  TAG_TILE_POLY = 12
###  TAG_TILE_LINE = 13

tileElement

## TILE ELEMENT

### TAG_ELEM_TAGS = 11

bytes : Varint32

0...N tagNum : Varint32

if tagNum >= TAGS_MAX :
        tagNum -= TAGS_LIMIT # custom tags

### TAG_ELEM_INDICES = 1

indexCount : Varint32


### TAG_ELEM_INDEX = 12

### TAG_ELEM_COORD = 13

### TAG_ELEM_LAYER = 21


* - Varint32 encoding

** - ZigZag encoding
