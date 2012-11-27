#!/usr/bin/env python

#
# protoc --python_out=. proto/TileData.proto
#

import sys
import proto.TileData_pb2

if __name__ == "__main__" :
    if len(sys.argv) != 2 :
        print>>sys.stderr, "Usage:", sys.argv[0], "<osmtile>"
        sys.exit(1)

    tile = proto.TileData_pb2.Data()

    try:
        f = open(sys.argv[1], "rb")
        tile.ParseFromString(f.read()[4:])
        f.close()
    except IOError:
        print sys.argv[1] + ": Could not open file.  Creating a new one."

    print tile
