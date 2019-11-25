#ifndef _DEFINE_PARSER_
#define _DEFINE_PARSER_

#include <string>
#include <cstdio>
#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <fstream>

using namespace std;

typedef pair<int, int> Pos;
typedef pair<Pos, Pos> NetPos;

class Parser
{
public:
    Parser(void) {}

    // specify the parsing file name and then parse the file
    bool read(char *inputFileName);

    // return # of tiles in horizontal direction
    int gNumHTiles() { return numHTilesm; }

    // return # of tiles in vertical direction
    int gNumVTiles() { return numVTilesm; }

    // return the capacity of each tile edge
    int gCapacity() { return capacity; }

    // return # of nets
    int gNumNets() { return numNets; }

    // return the starting x and y position for the net
    pair<int, int> gNetStart(int id) { return netsPos[id].first; }

    // return the ending x and y position for the net
    pair<int, int> gNetEnd(int id) { return netsPos[id].second; }

    // return net id
    int getNetId(const Pos &start, const Pos &end);

private:
    int numHTilesm, numVTilesm, capacity, numNets;
    vector<NetPos> netsPos;
    multimap<NetPos, int> netPosIdPair;
};

#endif
