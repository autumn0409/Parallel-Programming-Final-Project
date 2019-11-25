#include "parser.h"

bool Parser::read(char *inputFileName)
{
    ifstream infile(inputFileName);
    if (!infile)
    {
        cout << "Incorrect [input_file_name]: " << inputFileName << endl;
        cout << "Usage: ./parser [input_file_name]" << endl;
        return false;
    }

    string s, buf;
    while (infile >> s)
    {
        if (s == "grid")
        {
            infile >> numHTilesm >> numVTilesm;
        }
        else if (s == "capacity")
        {
            infile >> capacity;
        }
        else if (s == "num")
        {
            infile >> buf >> numNets;
            break;
        }
        else
        {
            cout << s << " does not follow the rule" << endl;
            break;
        }
    }
    netsPos.resize(numNets);
    for (size_t i = 0; i < numNets; ++i)
    {
        int id, sx, sy, ex, ey;
        infile >> id >> sx >> sy >> ex >> ey;
        Pos sPos = pair<int, int>(sx, sy);
        Pos ePos = pair<int, int>(ex, ey);
        NetPos newNet = pair<Pos, Pos>(sPos, ePos);
        netsPos[id] = newNet;
        netPosIdPair.insert(pair<NetPos, int>(newNet, id));
    }

    infile.close();
    return true;
}

int Parser::getNetId(const Pos &start, const Pos &end)
{
    NetPos targetNet = pair<Pos, Pos>(start, end);
    multimap<NetPos, int>::iterator it = netPosIdPair.find(targetNet);
    int targetId = it->second;

    if (netPosIdPair.count(targetNet) > 1)
    {
        netPosIdPair.erase(it); // delete one element
        NetPos originNet = pair<Pos, Pos>(start, end);
        netPosIdPair.insert(pair<NetPos, int>(originNet, targetId)); // insert again
    }

    return targetId;
}