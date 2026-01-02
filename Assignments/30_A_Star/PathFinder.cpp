#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

#include "AStar.h"
#include "PathFinder.h"

PathFinder::PathFinder() : startNode(nullptr), endNode(nullptr), currentNode(nullptr), isSearching(false),
                           isFound(false), isNoPath(false)
{
	Init();
}

PathFinder::~PathFinder() = default;

void PathFinder::Init()
{
	for (int y = 0; y < CELL_H; y++)
		for (int x = 0; x < CELL_W; x++)
		{
			grid[y][x].x = x;
			grid[y][x].y = y;
			grid[y][x].type = EMPTY;
			grid[y][x].Reset();
		}

	// SetStart(5, CELL_H / 2);
	// SetEnd(CELL_W - 6, CELL_H / 2);
}

void PathFinder::ResetSearch()
{
	pq = priority_queue<node*, vector<node*>, CompareNode>();
	isSearching = false;
	isFound = false;
	isNoPath = false;
	currentNode = nullptr;

	for (int y = 0; y < CELL_H; y++)
		for (int x = 0; x < CELL_W; x++)
			grid[y][x].Reset();

	if (startNode && endNode)
	{
		startNode->g = 0;
		startNode->h = GetManhattan(startNode->x, startNode->y, endNode->x, endNode->y);
		startNode->f = startNode->g + startNode->h;
		startNode->isOpen = true;
		isSearching = true;
		pq.emplace(startNode);
	}
}

void PathFinder::ClearWalls()
{
	for (int y = 0; y < CELL_H; y++)
		for (int x = 0; x < CELL_W; x++)
			if (grid[y][x].type == WALL)
				grid[y][x].type = EMPTY;

	ResetSearch();
}

void PathFinder::SetStart(int x, int y)
{
	if (x < 0 || CELL_W <= x || y < 0 || CELL_H <= y)
		return;

	if (startNode)
		startNode->type = EMPTY;

	startNode = &grid[y][x];
	startNode->type = START;
}

void PathFinder::SetEnd(int x, int y)
{
	if (x < 0 || CELL_W <= x || y < 0 || CELL_H <= y)
		return;

	if (endNode)
		endNode->type = EMPTY;

	endNode = &grid[y][x];
	endNode->type = END;
}

void PathFinder::ToggleWall(int x, int y)
{
	if (x < 0 || CELL_W <= x || y < 0 || CELL_H <= y)
		return;

	node* n = &grid[y][x];

	if (n->type == START || n->type == END)
		return;

	n->type = n->type == WALL ? EMPTY : WALL;
}

int PathFinder::GetManhattan(int x1, int y1, int x2, int y2)
{
	return (abs(x1 - x2) + abs(y1 - y2)) * 10;
}

int PathFinder::GetDistanceCost(int x1, int y1, int x2, int y2)
{
	// 유클리드 거리 근사: 직선 10, 대각선 14
	return x1 == x2 || y1 == y2 ? 10 : 14;
}

bool PathFinder::Step()
{
	if (!isSearching || isFound || isNoPath)
		return false;

	// 1. 큐가 빌 때까지 돌면서 유효한 탐색 대상 노드를 찾음 (Lazy Deletion)
	while (true)
	{
		if (pq.empty())
		{
			isNoPath = true;
			isSearching = false;
			return false;
		}

		currentNode = pq.top();
		pq.pop();

		if (currentNode->isClosed)
			continue;

		break;
	}

	currentNode->isOpen = false;
	currentNode->isClosed = true;

	// 2. 도착점 도달 체크
	if (currentNode == endNode)
	{
		isFound = true;
		isSearching = false;
		ReconstructPath();
		return false;
	}

	// 3. 8방향 탐색
	int dx[] = {0, 0, -1, 1, -1, -1, 1, 1};
	int dy[] = {-1, 1, 0, 0, -1, 1, -1, 1};

	for (int i = 0; i < 8; ++i)
	{
		int nx = currentNode->x + dx[i];
		int ny = currentNode->y + dy[i];

		if (nx < 0 || CELL_W <= nx || ny < 0 || CELL_H <= ny)
			continue;

		node* neighbor = &grid[ny][nx];

		if (neighbor->type == WALL)
			continue;

		int newG = currentNode->g + GetDistanceCost(currentNode->x, currentNode->y, nx, ny);

		// 4. 처음 방문한 노드거나, 새로운 경로가 더 짧다면 정보 갱신
		if (!neighbor->isOpen && !neighbor->isClosed || newG < neighbor->g)
		{
			neighbor->g = newG;
			neighbor->h = GetManhattan(nx, ny, endNode->x, endNode->y);
			neighbor->f = neighbor->g + neighbor->h;
			neighbor->parent = currentNode;
			neighbor->isClosed = false;
			neighbor->isOpen = true;

			pq.emplace(neighbor);
		}
	}

	return true;
}

void PathFinder::ReconstructPath() const
{
	node* curr = endNode;
	while (curr)
	{
		curr->isPath = true;
		curr = curr->parent;
	}
}
