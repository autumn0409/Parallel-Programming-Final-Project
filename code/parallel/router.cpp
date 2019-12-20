#include <stack>
#include <vector>
#include <algorithm>
#include <set>
#include "../util/parser.h"
#include "omp.h"

using namespace std;

class Vertex
{
public:
    Pos position;
    double g, f;
    Vertex *pi;
    int index;

    Vertex();
    Vertex(int x, int y, int gridNumX);
};

class Edge
{
public:
    int vertexIndex;
    double flowNum;
    double weight;

    Edge(int num);
};

void relax(Vertex &v, int j, const Pos &end, set<Vertex> &openSet, set<Vertex> &closedSet,
           vector<Vertex> &vertices, const vector<vector<Edge>> &adjList);
int getEdgeIndex(int listIndex, const Vertex *v, vector<vector<Edge>> &adjList);
void updateEdgeInfo(const Vertex *vNow, vector<vector<Edge>> &adjList);
int calcHeuristic(const Pos &posNow, const Pos &goal);
void initSrc(const Pos &start, const Pos &end, vector<Vertex> &vertices);
int pairToIndex(const Pos &position);
void setGraph(int gridNumX, int gridNumY, int capacity, int netCnt, vector<vector<Edge>> &adjList,
              vector<Vertex> &vertices);
void routing(const Pos &start, const Pos &end, vector<Vertex> &vertices, const vector<vector<Edge>> &adjList);
// store path and update edge weight and flowNum
stack<Pos> getPath(const Pos &start, const Pos &end, vector<Vertex> &vertices, vector<vector<Edge>> &adjList);
void printRoutes(stack<Pos> &routingPath, Parser &p, ofstream &output);

bool operator<(const Vertex &, const Vertex &);
bool operator>(const Vertex &, const Vertex &);
bool operator==(const Vertex &v1, const Vertex &v2);

int gridNumX, capacity, netCnt;

// ================ main funtion =================
int main(int argc, char **argv)
{
    int threadCnt = atoi(argv[3]);
    omp_set_num_threads(threadCnt);
    ofstream output(argv[2]);

    Parser p;
    vector<vector<Edge>> adjList;
    vector<Vertex> vertices;

    p.read(argv[1]);
    gridNumX = p.gNumHTiles();
    capacity = p.gCapacity();
    netCnt = p.gNumNets();

    setGraph(gridNumX, p.gNumVTiles(), capacity, netCnt, adjList, vertices);

#pragma omp parallel shared(adjList, p, output) firstprivate(vertices)
    {
        vector<stack<Pos>> paths;
        stack<Pos> path;

        int rank = omp_get_thread_num();
        int parallelNetCnt = netCnt / threadCnt * threadCnt; // # of nets routed parallelly

        for (int i = 0 + rank; i < parallelNetCnt; i = i + threadCnt)
        {
            Pos start = p.gNetStart(i);
            Pos end = p.gNetEnd(i);
            routing(start, end, vertices, adjList);

#pragma omp barrier
            path = getPath(start, end, vertices, adjList);
#pragma omp barrier

            paths.push_back(path);
        }

#pragma omp master
        for (int i = parallelNetCnt; i < netCnt; i++) // rest of nets are routed by master thread
        {
            Pos start = p.gNetStart(i);
            Pos end = p.gNetEnd(i);
            routing(start, end, vertices, adjList);
            path = getPath(start, end, vertices, adjList);
            paths.push_back(path);
        }

#pragma omp critical
        for (int j = 0; j < paths.size(); j++)
        {
            printRoutes(paths[j], p, output);
        }
    }

    output.close();
    return 0;
}
// =================================================

// Vertex
Vertex::Vertex() : g(-1), f(-1), pi(nullptr), index(-1)
{
    this->position.first = -1;
    this->position.second = -1;
}
Vertex::Vertex(int x, int y, int gridNumX) : g(-1), f(-1), pi(nullptr)
{
    this->position.first = x;
    this->position.second = y;
    this->index = x + gridNumX * y;
}

// Vertex operator overloading
bool operator<(const Vertex &v1, const Vertex &v2)
{
    if (v1.f != v2.f)
        return v1.f < v2.f;
    else
        return v1.index < v2.index;
}
bool operator>(const Vertex &v1, const Vertex &v2)
{
    if (v1.f != v2.f)
        return v1.f > v2.f;
    else
        return v1.index > v2.index;
}
bool operator==(const Vertex &v1, const Vertex &v2)
{
    return v1.index == v2.index;
}

// Edge
Edge::Edge(int num) : vertexIndex(num), weight(1), flowNum(0)
{
}

// global function
void setGraph(int gridNumX, int gridNumY, int capacity, int netCnt, vector<vector<Edge>> &adjList,
              vector<Vertex> &vertices)
{
    for (int j = 0; j < gridNumY; j++)
    {
        for (int i = 0; i < gridNumX; i++)
        {
            vector<Edge> edges;
            Vertex aVertex(i, j, gridNumX);
            int index = i + gridNumX * j;

            if (i < gridNumX - 1) // The vertexPos now has a right edge.
            {
                Edge rightEdge(index + 1);
                edges.push_back(rightEdge);
            }

            if (i > 0) // The vertexPos now has a left edge.
            {
                Edge leftEdge(index - 1);
                edges.push_back(leftEdge);
            }

            if (j < gridNumY - 1) // The vertexPos now has a upper edge.
            {
                Edge upperEdge(index + gridNumX);
                edges.push_back(upperEdge);
            }

            if (j > 0) // The vertexPos now has a lower edge.
            {
                Edge lowerEdge(index - gridNumX);
                edges.push_back(lowerEdge);
            }

            adjList.push_back(edges);
            vertices.push_back(aVertex);
        }
    }
}
void routing(const Pos &start, const Pos &end, vector<Vertex> &vertices, const vector<vector<Edge>> &adjList)
{
    // A* search algorithm
    set<Vertex> closedSet;
    set<Vertex> openSet;

    int srcIndex = pairToIndex(start);
    initSrc(start, end, vertices);
    openSet.insert(vertices[srcIndex]);
    while (!openSet.empty())
    {
        Vertex u = *openSet.begin();    // extract Vertex who's f is minimum from openSet
        openSet.erase(openSet.begin()); // remove u from openSet

        if (u.position == end)
            break;

        closedSet.insert(u);

        int uDeg = adjList[u.index].size(); // # of Vertex around u
        for (int j = 0; j < uDeg; j++)
            relax(u, j, end, openSet, closedSet, vertices, adjList);
    }
}
stack<Pos> getPath(const Pos &start, const Pos &end, vector<Vertex> &vertices, vector<vector<Edge>> &adjList)
{
    stack<Pos> routingPath;
    int endIndex = pairToIndex(end);
    Vertex *vNow = &(vertices[endIndex]);

    routingPath.push(vNow->position);
    while (vNow->pi != nullptr)
    {
#pragma omp critical
        updateEdgeInfo(vNow, adjList);

        routingPath.push(vNow->pi->position);
        vNow = vNow->pi;
    }

    routingPath.push(end);
    routingPath.push(start);

    return routingPath;
}
void relax(Vertex &u, int j, const Pos &end, set<Vertex> &openSet, set<Vertex> &closedSet,
           vector<Vertex> &vertices, const vector<vector<Edge>> &adjList)
{
    // get the weight of the edge between u and v
    double weight = adjList[u.index][j].weight;

    // get Vertex v from u and j
    int vIndex = adjList[u.index][j].vertexIndex;
    Vertex *v = &(vertices[vIndex]);

    if (closedSet.find(*v) != closedSet.end()) // v is in the closedSet
        return;

    bool tentativeIsBetter;
    if (openSet.find(*v) == openSet.end()) // v is not in the openSet
        tentativeIsBetter = true;
    else if (v->g > u.g + weight)
        tentativeIsBetter = true;
    else
        tentativeIsBetter = false;

    if (tentativeIsBetter)
    {
        v->pi = &(vertices[u.index]);
        v->g = u.g + weight;
        v->f = v->g + calcHeuristic(v->position, end);
        openSet.insert(vertices[vIndex]);
    }
}
int getEdgeIndex(int listIndex, const Vertex *v, vector<vector<Edge>> &adjList)
{
    int i = 0;
    while (adjList[listIndex][i].vertexIndex != v->index)
        i++;

    return i;
}
void initSrc(const Pos &start, const Pos &end, vector<Vertex> &vertices)
{
    // initialize all Vertex's g, f and pi in the map
    for (int i = 0; i < vertices.size(); i++)
    {
        vertices[i].g = INT32_MAX;
        vertices[i].f = INT32_MAX;
        vertices[i].pi = nullptr;
    }
    // initialize source Vertex's g and f
    int h = calcHeuristic(start, end);
    vertices[pairToIndex(start)].g = 0;
    vertices[pairToIndex(start)].f = h;
}
void updateEdgeInfo(const Vertex *vNow, vector<vector<Edge>> &adjList)
{
    // vNow's egde index in vNow->pi's list
    int vNow_edgeIndex = getEdgeIndex(vNow->pi->index, vNow, adjList);

    // vNow->pi's edge index in vNow's list
    int vNowPi_edgeIndex = getEdgeIndex(vNow->index, vNow->pi, adjList);

    // derive new flowNum & new weight
    double newFlowNum = adjList[vNow->pi->index][vNow_edgeIndex].flowNum + 1;
    double newWeight = 1 + (newFlowNum >= capacity ? pow(2, newFlowNum / capacity) : newFlowNum / capacity);

    // update flowNum and weight
    adjList[vNow->pi->index][vNow_edgeIndex].weight = newWeight;
    adjList[vNow->index][vNowPi_edgeIndex].weight = newWeight;
    adjList[vNow->pi->index][vNow_edgeIndex].flowNum = newFlowNum;
    adjList[vNow->index][vNowPi_edgeIndex].flowNum = newFlowNum;
}
int pairToIndex(const Pos &position)
{
    int x = position.first;
    int y = position.second;
    return x + gridNumX * y;
}
int calcHeuristic(const Pos &posNow, const Pos &goal)
{
    return abs(posNow.first - goal.first) + abs(posNow.second - goal.second);
}
void printRoutes(stack<Pos> &routingPath, Parser &p, ofstream &output)
{
    Pos start = routingPath.top();
    routingPath.pop();
    Pos end = routingPath.top();
    routingPath.pop();

    int routingDist = routingPath.size() - 1;
    output << p.getNetId(start, end) << " " << routingDist << endl;

    while (routingPath.size() > 1)
    {
        output << routingPath.top().first << " " << routingPath.top().second << " ";
        routingPath.pop();
        output << routingPath.top().first << " " << routingPath.top().second << endl;
    }
}
