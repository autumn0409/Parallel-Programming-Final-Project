#include "parser.h"
#include <algorithm>

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
    sort(netsPos.begin(), netsPos.end(), cmp);
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

bool cmp(const NetPos &n1, const NetPos &n2)
{
    int boundingSize1 = boundingSize(n1.first, n1.second);
    int boundingSize2 = boundingSize(n2.first, n2.second);
    if (boundingSize1 == boundingSize2)
        return distance(n1.first, n1.second) < distance(n2.first, n2.second);
    else
        return boundingSize1 < boundingSize2;
}

int boundingSize(const Pos &start, const Pos &end)
{
    return abs(start.first - end.first) * abs(start.second - end.second);
}

int distance(const Pos &start, const Pos &end)
{
    return pow(start.first - end.first, 2) + pow(start.second - end.second, 2);
}
