#include <stack>
#include <queue>
#include "parser.h"
#include "timer.h"

using namespace std;

class Vertex
{
private:
    Pos position;
    double d;
    Vertex *pi;
    int index;

public:
    Vertex();
    Vertex(int x, int y, int gridNumX);

    friend bool operator<(const Vertex &, const Vertex &);
    friend bool operator>(const Vertex &, const Vertex &);
    friend class Graph;
};

class Edge
{
private:
    int vertexIndex;
    double flowNum;
    double weight;

public:
    Edge(int num);

    friend class Graph;
};

class Graph
{
private:
    int gridNumX;
    int gridNumY;
    int capacity;
    int netCnt;
    vector<vector<Edge>> adjList;
    vector<Vertex> vertices;

    int getEdgeIndex(int listIndex, const Vertex *v);
    void initSrc(const Pos &start);
    void relax(Vertex &v, priority_queue<Vertex> &Q, int j);
    void updateEdgeInfo(const Vertex *vNow);
    void printRoutes(const Pos &start, const Pos &end, int i, ofstream &output);
    int pairToIndex(const Pos &position);

public:
    Graph();
    void setGraph(int gridNumX, int gridNumY, int capacity, int netCnt);
    void routing(Parser &p, int i, ofstream &output);
};

// main funtion
int main(int argc, char **argv)
{
    Parser p;
    Timer t;
    Graph map;
    ofstream output(argv[2]);

    t.Begin();
    p.read(argv[1]);
    map.setGraph(p.gNumHTiles(), p.gNumVTiles(), p.gCapacity(), p.gNumNets());

    for (int i = 0; i < p.gNumNets(); i++)
        map.routing(p, i, output);

    cout << "Execution time: " << t.End() << "s" << endl;
    output.close();
    return 0;
}

// Vertex
Vertex::Vertex() : d(-1), pi(nullptr), index(-1)
{
    this->position.first = -1;
    this->position.second = -1;
}
Vertex::Vertex(int x, int y, int gridNumX) : d(-1), pi(nullptr)
{
    this->position.first = x;
    this->position.second = y;
    this->index = x + gridNumX * y;
}

// Vertex operator overloading
bool operator<(const Vertex &v1, const Vertex &v2)
{
    return v1.d > v2.d;
}
bool operator>(const Vertex &v1, const Vertex &v2)
{
    return v1.d < v2.d;
}

// Edge
Edge::Edge(int num) : vertexIndex(num), weight(0), flowNum(0)
{
}

// Graph
Graph::Graph() : gridNumX(0), gridNumY(0), capacity(0), netCnt(0)
{
}
void Graph::setGraph(int gridNumX, int gridNumY, int capacity, int netCnt)
{
    this->gridNumX = gridNumX;
    this->gridNumY = gridNumY;
    this->capacity = capacity;
    this->netCnt = netCnt;

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

            this->adjList.push_back(edges);
            this->vertices.push_back(aVertex);
        }
    }
}
void Graph::routing(Parser &p, int i, ofstream &output)
{
    Pos start = p.gNetStart(i);
    Pos end = p.gNetEnd(i);
    priority_queue<Vertex> Q;
    int srcIndex = this->pairToIndex(start);

    this->initSrc(start);
    Q.push(this->vertices[srcIndex]);

    while (!Q.empty())
    {
        Vertex u = Q.top();
        Q.pop();

        int uDeg = this->adjList[u.index].size();
        for (int j = 0; j < uDeg; j++)
            this->relax(u, Q, j);
    }

    this->printRoutes(start, end, i, output);
}
int Graph::getEdgeIndex(int listIndex, const Vertex *v)
{
    int i = 0;
    while (this->adjList[listIndex][i].vertexIndex != v->index)
        i++;

    return i;
}
void Graph::initSrc(const Pos &start)
{
    for (int i = 0; i < this->vertices.size(); i++)
    {
        this->vertices[i].d = INT32_MAX;
        this->vertices[i].pi = nullptr;
    }
    this->vertices[this->pairToIndex(start)].d = 0;
}
void Graph::relax(Vertex &u, priority_queue<Vertex> &Q, int j)
{
    double weight = this->adjList[u.index][j].weight;

    int vIndex = this->adjList[u.index][j].vertexIndex;
    Vertex *v = &(this->vertices[vIndex]);

    if (v->d > u.d + weight)
    {
        v->d = u.d + weight;
        v->pi = &(this->vertices[u.index]);
        Q.push(this->vertices[vIndex]);
    }
}
void Graph::printRoutes(const Pos &start, const Pos &end, int i, ofstream &output)
{
    stack<Pos> S;
    int routesCnt = 0;
    int endIndex = this->pairToIndex(end);

    Vertex *vNow = &(this->vertices[endIndex]);
    S.push(vNow->position);

    while (vNow->pi != nullptr)
    {
        this->updateEdgeInfo(vNow);

        S.push(vNow->pi->position);
        vNow = vNow->pi;

        routesCnt++;
    }

    output << i << " " << routesCnt << endl;
    while (S.size() > 1)
    {
        output << S.top().first << " " << S.top().second << " ";
        S.pop();
        output << S.top().first << " " << S.top().second << endl;
    }
}
void Graph::updateEdgeInfo(const Vertex *vNow)
{
    // vNow's egde index in vNow->pi's list
    int vNow_edgeIndex = this->getEdgeIndex(vNow->pi->index, vNow);

    // vNow->pi's edge index in vNow's list
    int vNowPi_edgeIndex = this->getEdgeIndex(vNow->index, vNow->pi);

    // derive new flowNum & new weight
    double newFlowNum = this->adjList[vNow->pi->index][vNow_edgeIndex].flowNum + 1;
    double newWeight = newFlowNum / this->capacity;

    // update flowNum and weight
    this->adjList[vNow->pi->index][vNow_edgeIndex].weight = newWeight;
    this->adjList[vNow->index][vNowPi_edgeIndex].weight = newWeight;
    this->adjList[vNow->pi->index][vNow_edgeIndex].flowNum = newFlowNum;
    this->adjList[vNow->index][vNowPi_edgeIndex].flowNum = newFlowNum;
}
int Graph::pairToIndex(const Pos &position)
{
    int x = position.first;
    int y = position.second;
    return x + this->gridNumX * y;
}