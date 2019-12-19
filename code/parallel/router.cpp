#include <stack>
#include <vector>
#include <algorithm>
#include <set>
#include "../util/parser.h"
#include "../util/timer.h"
#include <pthread.h>

using namespace std;

pthread_mutex_t mutex;

typedef struct thread_data
{
    int rank;
    int threadCount;
    char *outputName;
} thread_data;

class Vertex
{
    friend class Graph;
    friend bool operator<(const Vertex &, const Vertex &);
    friend bool operator>(const Vertex &, const Vertex &);
    friend bool operator==(const Vertex &v1, const Vertex &v2);

private:
    Pos position;
    double g, f;
    Vertex *pi;
    int index;

public:
    Vertex();
    Vertex(int x, int y, int gridNumX);
};

class Edge
{
    friend class Graph;

private:
    int vertexIndex;
    double flowNum;
    double weight;

public:
    Edge(int num);
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
    void initSrc(const Pos &start, const Pos &end);
    void relax(Vertex &v, int j, const Pos &end, set<Vertex> &openSet, set<Vertex> &closedSet);
    void updateEdgeInfo(const Vertex *vNow);
    int pairToIndex(const Pos &position);
    int calcHeuristic(const Pos &posNow, const Pos &goal);

public:
    Graph(int gridNumX, int gridNumY, int capacity, int netCnt);
    stack<Pos> routing(const Pos &start, const Pos &end);
};

void printRoutes(stack<Pos> &routingPath, Parser &p, ofstream &output);

void *Thread_routing(void *param);

Parser p;

// ================ main funtion =================
int main(int argc, char **argv)
{
    int thread_count = atoi(argv[3]);
    Timer t;
    ofstream output(argv[2]);
    output.close();
    pthread_t th[thread_count];
    pthread_mutex_init(&mutex, NULL);
    thread_data data[thread_count];

    t.Begin();

    p.read(argv[1]);
    for (int i = 0; i < thread_count; i++)
    {
        data[i].rank = i;
        data[i].threadCount = thread_count;
        data[i].outputName = argv[2];
        pthread_create(&th[i], NULL, Thread_routing, (void *)&data[i]);
    }

    for (int i = 0; i < thread_count; i++)
    {
        pthread_join(th[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    cout << "Execution time: " << t.End() << "s" << endl;
    return 0;
}
// =================================================

// Thread_function
void *Thread_routing(void *param)
{
    thread_data *data = (thread_data *)param;

    int rank = data->rank;
    int threadCount = data->threadCount;
    char *outputName = data->outputName;
    int startI, endI;
    vector<stack<Pos>> paths;

    ofstream write;
    write.open(outputName, ios::app);
    Graph map(p.gNumHTiles(), p.gNumVTiles(), p.gCapacity(), p.gNumNets());

    for (int i = 0 + rank; i < p.gNumNets(); i = i + threadCount)
    {
        Pos start = p.gNetStart(i);
        Pos end = p.gNetEnd(i);
        paths.push_back(map.routing(start, end));
    }

    pthread_mutex_lock(&mutex);
    for (int j = 0; j < paths.size(); j++)
    {
        printRoutes(paths[j], p, write);
    }
    pthread_mutex_unlock(&mutex);

    write.close();
    return NULL;
}

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

// Graph
Graph::Graph(int gridNumX, int gridNumY, int capacity, int netCnt)
    : gridNumX(gridNumX), gridNumY(gridNumY), capacity(capacity), netCnt(netCnt)
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

            this->adjList.push_back(edges);
            this->vertices.push_back(aVertex);
        }
    }
}
stack<Pos> Graph::routing(const Pos &start, const Pos &end)
{
    // A* search algorithm
    set<Vertex> closedSet;
    set<Vertex> openSet;

    int srcIndex = this->pairToIndex(start);
    this->initSrc(start, end);
    openSet.insert(this->vertices[srcIndex]);

    while (!openSet.empty())
    {
        Vertex u = *openSet.begin();    // extract Vertex who's f is minimum from openSet
        openSet.erase(openSet.begin()); // remove u from openSet

        if (u.position == end)
            break;

        closedSet.insert(u);

        int uDeg = this->adjList[u.index].size(); // # of Vertex around u
        for (int j = 0; j < uDeg; j++)
            this->relax(u, j, end, openSet, closedSet);
    }

    // store path and update edge weight and flowNum
    stack<Pos> routingPath;
    int endIndex = this->pairToIndex(end);
    Vertex *vNow = &(this->vertices[endIndex]);

    routingPath.push(vNow->position);
    while (vNow->pi != nullptr)
    {
        this->updateEdgeInfo(vNow);
        routingPath.push(vNow->pi->position);
        vNow = vNow->pi;
    }
    routingPath.push(end);
    routingPath.push(start);

    return routingPath;
}
void Graph::relax(Vertex &u, int j, const Pos &end, set<Vertex> &openSet, set<Vertex> &closedSet)
{
    // get the weight of the edge between u and v
    double weight = this->adjList[u.index][j].weight;

    // get Vertex v from u and j
    int vIndex = this->adjList[u.index][j].vertexIndex;
    Vertex *v = &(this->vertices[vIndex]);

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
        v->pi = &(this->vertices[u.index]);
        v->g = u.g + weight;
        v->f = v->g + calcHeuristic(v->position, end);
        openSet.insert(this->vertices[vIndex]);
    }
}
int Graph::getEdgeIndex(int listIndex, const Vertex *v)
{
    int i = 0;
    while (this->adjList[listIndex][i].vertexIndex != v->index)
        i++;

    return i;
}
void Graph::initSrc(const Pos &start, const Pos &end)
{
    // initialize all Vertex's g, f and pi in the map
    for (int i = 0; i < this->vertices.size(); i++)
    {
        this->vertices[i].g = INT32_MAX;
        this->vertices[i].f = INT32_MAX;
        this->vertices[i].pi = nullptr;
    }
    // initialize source Vertex's g and f
    int h = this->calcHeuristic(start, end);
    this->vertices[this->pairToIndex(start)].g = 0;
    this->vertices[this->pairToIndex(start)].f = h;
}
void Graph::updateEdgeInfo(const Vertex *vNow)
{
    // vNow's egde index in vNow->pi's list
    int vNow_edgeIndex = this->getEdgeIndex(vNow->pi->index, vNow);

    // vNow->pi's edge index in vNow's list
    int vNowPi_edgeIndex = this->getEdgeIndex(vNow->index, vNow->pi);

    // derive new flowNum & new weight
    double newFlowNum = this->adjList[vNow->pi->index][vNow_edgeIndex].flowNum + 1;
    double newWeight = 1 + newFlowNum / this->capacity;

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
int Graph::calcHeuristic(const Pos &posNow, const Pos &goal)
{
    return abs(posNow.first - goal.first) + abs(posNow.second - goal.second);
}

// global function
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
