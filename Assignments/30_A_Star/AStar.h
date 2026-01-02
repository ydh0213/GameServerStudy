#pragma once

constexpr int WIDTH = 1800;
constexpr int HEIGHT = 1000;
constexpr int CELL_SIZE = 20;
constexpr int CELL_W = WIDTH / CELL_SIZE;
constexpr int CELL_H = HEIGHT / CELL_SIZE;

enum NodeType { EMPTY, WALL, START, END };

struct node
{
	int x, y;
	node* parent;
	int g; // 출발점으로부터의 이동거리 (유클리드 거리 권장)
	int h; // 목적지와의 절대 거리 (맨해튼 거리 권장)
	int f; // f = g + h (f가 가장 작은 노드부터 검색)

	bool isOpen;
	bool isClosed;
	bool isPath;
	NodeType type;

	void Reset()
	{
		parent = nullptr;
		g = 0;
		h = 0;
		f = 0;
		isOpen = false;
		isClosed = false;
		isPath = false;
	}
};
