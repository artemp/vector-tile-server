#!/usr/bin/env python

import sys

def signed(val):
    if val & ~0x7f == 0x80 :
        return val|~0x7f
    else:
        return val&0x7f

class vector_tile:

    TAGS_MAX = 627
    TAGS_LIMIT = 1024
    tags = {}

    def __init__(self, buf) :
        self.buf = buf
        self.pos = 0

    def decodeSize (self):
        self.pos += 4
        return self.buf[0] << 24 | self.buf[1] << 16 | self.buf[2] << 8 | self.buf[3]

    def decodeVarint32 (self):
        pos = self.pos
        #print>>sys.stderr,"bUFFER=>",self.buf[pos],self.buf[pos+1],self.buf[pos+2],self.buf[pos+3],self.buf[pos+4]
        if signed(self.buf[pos]) >= 0:
            self.pos +=1
            return self.buf[pos]
        elif signed(self.buf[pos+1]) >= 0:
            self.pos +=2
            return (self.buf[pos] & 0x7f) | (self.buf[pos+1] << 7)
        elif signed(self.buf[pos+2]) >= 0:
            self.pos +=3
            return (self.buf[pos] & 0x7f) | (self.buf[pos + 1] & 0x7f) << 7 | (self.buf[pos + 2]) << 14
        elif signed(self.buf[pos+3]) >= 0:
            self.pos +=4
            return (self.buf[pos] & 0x7f) | (self.buf[pos + 1] & 0x7f) << 7 | (self.buf[pos + 2] & 0x7f) << 14 | (self.buf[pos + 3]) << 21

        result = (self.buf[pos] & 0x7f) | (self.buf[pos + 1] & 0x7f) << 7 | (self.buf[pos + 2] & 0x7f) << 14 | (self.buf[pos + 3] & 0x7f) << 21 | (self.buf[pos + 4]) << 28
        print "--------------------------------------OOOPS"
        read = 5
        pos += 4
        while buf[pos]<0 and read < 10:
            pos+=1
            read+=1

        self.pos += read
        return result

    def decodeShortArray(self, numTags):
         bytes = self.decodeVarint32()
         print>>sys.stderr, "BYTES=", bytes
         end = self.pos + bytes
         index = []
         while self.pos < end :
             val = self.decodeVarint32()
             index.append(val)
         return index

    def decodeTileTags(self, tag, index):
        val = self.decodeString()
        print "key=",tag ," val=",val
        self.tags[tag]=val

    def decodeString(self):
        size = self.decodeVarint32()
        pos = self.pos
        self.pos += size
        return self.buf[pos:pos + size]

    def decodeTileElement(self, type, index):
        bytes = self.decodeVarint32()
        end = self.pos + bytes
        indexCount = 1
        coordCount = 0
        if (type == 13):
            coordCount = 2

        while self.pos < end:
            val = self.decodeVarint32()
            #print "VAL=",val
            if val==0: break
            tag = val >> 3;
            print>>sys.stderr, "POS=", self.pos, "VAL=", val, "TAG=", tag
            if tag == 11 : #TAG_ELEM_TAGS
                print "TAG_ELEM_TAGS"
                self.decodeWayTags(index)
            elif tag == 1: # TAG_ELEM_NUM_INDICES
                print "TAG_ELEM_NUM_INDICES"
                indexCount = self.decodeVarint32();
                print "INDEX Count = ",indexCount
                #sys.exit(1)
            elif tag == 12: # TAG_ELEM_INDEX
                print "TAG_ELEM_INDEX"
                print "indexCount=", indexCount
                index = self.decodeShortArray(indexCount)
                print "INDEX:",index
                for i in range(0,indexCount):
                    length = index[i]*2
                    coordCount += length
                    index[i] = length
                if indexCount < len(index):
                    index[indexCount] = -1
            elif tag == 13: # TAG_ELEM_COORDS
                print "TAG_ELEM_COORDS"
                count = self.decodeWayCoordinates(coordCount)
                print "COUNT=",count, " --- " , coordCount
            elif tag == 21: # TAG_ELEM_LAYER
                print "TAG_ELEM_LAYER"
                sys.exit(1)
            else:
                print>>sys.stderr,"Invalid type for element:", tag
                sys.exit(1)

    def decodeWayTags(self,index):
        bytes = self.decodeVarint32()
        end = self.pos + bytes
        while self.pos < end:
            tagNum = self.decodeVarint32()
            if tagNum < self.TAGS_MAX:
                print "TagNum=", tagNum
            else :
                tagNum -= self.TAGS_LIMIT
                print "TagNum(Local)=", index[tagNum], "val=", self.tags[tagNum]


    def decodeWayCoordinates(self, nodes ):
        bytes = self.decodeVarint32()
        end = self.pos + bytes
        last_x = 0
        last_y = 0
        count = 0
        even = True
        scale = 4096/400.0
        while self.pos < end:
            val = self.decodeVarint32()
            val = ((val >> 1) ^ -(val & 1)) #zigzag
            count +=1
            if even :
                last_x += val
                even = False
                print "X => ", last_x/scale
            else :
                last_y += val
                even = True
                print "Y => ", last_y/scale
        return count

def parse(filename):
    f = open(filename)
    s = f.read()
    tile = vector_tile(bytearray(s))
    print "content length: ", tile.decodeSize()
    val = tile.decodeVarint32()
    numTags = 0
    curTag = 0
    index = []
    while tile.pos < len(s):
        tag = val >> 3
        print>>sys.stderr, "POS=", tile.pos, "VAL=", val, "TAG=", tag
        if tag == 1 : # TAG_TILE_NUM_TAGS
            numTags = tile.decodeVarint32()
        elif tag == 2 : # TAG_TILE_TAG_KEYS
            index = tile.decodeShortArray(numTags)
            print "INDEX:",index
        elif tag == 3 : # TAG_TILE_TAG_VALUES:
            tile.decodeTileTags(curTag,index);
            curTag += 1
        elif tag == 11 or tag == 12 or tag == 13 : # TAG_TILE_LINE/POLY/POINT
            print " TAG_TILE_LINE/POLY/POINT"
            tile.decodeTileElement(tag, index);
        else:
            print>>sys.stderr,"Invalid type for tile:", tag
            break
        if (tile.pos < len(s)):
            val = tile.decodeVarint32()

if __name__ == "__main__":

    if len(sys.argv) != 2 :
        print>>sys.stderr, "Usage:", sys.argv[0], "<osmtile>"
        sys.exit(1)
    parse(sys.argv[1])
