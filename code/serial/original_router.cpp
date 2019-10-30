#include <stack>
#include <queue>
#include <ctime>
#include <string>
#include <cstdio>
#include <iostream>
#include <utility>
#include <vector>
#include <fstream>

using namespace std;
typedef pair<int, int> Pos;

class AlgParser
{
  public:
	AlgParser(void) {}

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

  private:
	int numHTilesm, numVTilesm, capacity, numNets;
	typedef pair<int, int> Pos;
	typedef pair<Pos, Pos> NetPos;
	vector<NetPos> netsPos;
};

class AlgTimer
{
  public:
	AlgTimer(void) {}

	// Strat the timer
	void Begin(void);

	// Return the accumulated time in seconds
	double End(void);

  private:
	long begin_clock;
};

//

class Vertex
{
  public:
	Pos position;
	double d;
	Vertex *pi;
	int index;

	Vertex();
	Vertex(int x, int y);
};

class Edge
{
  public:
	int vertexIndex;
	double flowNum;
	double weight;

	Edge(int num);
};

class Graph
{
  public:
	int gridNumX;
	int gridNumY;
	int capacity;
	int netCnt;
	vector<vector<Edge>> adjList;
	vector<Vertex> vertices;

	Graph();
	void setGraph(int gridNumX, int gridNumY, int capacity, int netCnt);
	int getEdgeIndex(int listIndex, Vertex *v);
};

// global function headers

int pairToIndex(const Pos &position);
void initSrc(const Pos &start);
int routing(AlgParser &p, int i, ofstream &output);
void relax(Vertex &v, priority_queue<Vertex> &Q, int j);
void printRoutes(const Pos &start, const Pos &end, int i, ofstream &output);

bool operator<(const Vertex &, const Vertex &);
bool operator>(const Vertex &, const Vertex &);

//

Graph map;

//==============================================================================================

// main funtion

int main(int argc, char **argv)
{

	AlgParser p;
	// AlgTimer t;

	p.read(argv[1]);
	// t.Begin();
	ofstream output(argv[2]);

	map.setGraph(p.gNumHTiles(), p.gNumVTiles(), p.gCapacity(), p.gNumNets());

	for (int i = 0; i < map.netCnt; i++)
		routing(p, i, output);

	// cout << "The execution spends " << t.End() << " seconds" << endl;
	output.close();
	return 0;
}

//================================================================================================

//

bool AlgParser::read(char *inputFileName)
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
		netsPos[id] = pair<Pos, Pos>(sPos, ePos);
	}
	infile.close();
	// cout << "file parsed completely." << endl;
	return true;
}

void AlgTimer::Begin(void)
{
	begin_clock = clock();
}

double AlgTimer::End(void)
{
	return ((double)(clock() - begin_clock) / (double)CLOCKS_PER_SEC);
}

//

Vertex::Vertex() : d(-1), pi(nullptr), index(-1)
{
	this->position.first = -1;
	this->position.second = -1;
}

Vertex::Vertex(int x, int y) : d(-1), pi(nullptr)
{
	this->position.first = x;
	this->position.second = y;
	this->index = x + map.gridNumX * y;
}

//

Edge::Edge(int num) : vertexIndex(num), weight(0), flowNum(0)
{
}

//

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
			Vertex aVertex(i, j);
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

int Graph::getEdgeIndex(int listIndex, Vertex *v)
{
	int i = 0;
	while (this->adjList[listIndex][i].vertexIndex != v->index)
		i++;

	return i;
}

// global functions

int pairToIndex(const Pos &position)
{
	int x = position.first;
	int y = position.second;
	return x + map.gridNumX * y;
}

void initSrc(const Pos &start)
{
	for (int i = 0; i < map.vertices.size(); i++)
	{
		map.vertices[i].d = INT32_MAX;
		map.vertices[i].pi = nullptr;
	}
	map.vertices[pairToIndex(start)].d = 0;
}

int routing(AlgParser &p, int i, ofstream &output)
{
	Pos start = p.gNetStart(i);
	Pos end = p.gNetEnd(i);
	priority_queue<Vertex> Q;
	int srcIndex = pairToIndex(start);

	initSrc(start);
	Q.push(map.vertices[srcIndex]);

	while (!Q.empty())
	{
		Vertex u = Q.top();
		Q.pop();

		int uDeg = map.adjList[u.index].size();
		for (int j = 0; j < uDeg; j++)
			relax(u, Q, j);
	}

	printRoutes(start, end, i, output);
}

void relax(Vertex &u, priority_queue<Vertex> &Q, int j)
{
	double weight = map.adjList[u.index][j].weight;

	int vIndex = map.adjList[u.index][j].vertexIndex;
	Vertex *v = &map.vertices[vIndex];

	if (v->d > u.d + weight)
	{
		v->d = u.d + weight;
		v->pi = &map.vertices[u.index];
		Q.push(map.vertices[vIndex]);
	}
}

void printRoutes(const Pos &start, const Pos &end, int i, ofstream &output)
{
	stack<Pos> S;
	int routesCnt = 0;
	int endIndex = pairToIndex(end);

	Vertex *vNow = &map.vertices[endIndex];
	S.push(vNow->position);

	while (vNow->pi != nullptr)
	{
		// vNow's egde index in vNow->pi's list
		int vNow_edgeIndex = map.getEdgeIndex(vNow->pi->index, vNow);

		// vNow->pi's edge index in vNow's list
		int vNowPi_edgeIndex = map.getEdgeIndex(vNow->index, vNow->pi);

		// derive new flowNum & new weight
		double newFlowNum = map.adjList[vNow->pi->index][vNow_edgeIndex].flowNum + 1;
		double newWeight = newFlowNum / map.capacity;

		// update flowNum and weight
		map.adjList[vNow->pi->index][vNow_edgeIndex].weight = newWeight;
		map.adjList[vNow->index][vNowPi_edgeIndex].weight = newWeight;
		map.adjList[vNow->pi->index][vNow_edgeIndex].flowNum = newFlowNum;
		map.adjList[vNow->index][vNowPi_edgeIndex].flowNum = newFlowNum;

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

bool operator<(const Vertex &v1, const Vertex &v2)
{
	return v1.d > v2.d;
}

bool operator>(const Vertex &v1, const Vertex &v2)
{
	return v1.d < v2.d;
}
