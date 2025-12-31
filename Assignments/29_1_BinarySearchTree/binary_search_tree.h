#pragma once

constexpr int RADIUS = 15; // 노드 반지름
constexpr int HOR_GAP = 20; // 노드 간 가로 간격
constexpr int VER_GAP = 60; // 노드 간 세로 간격

struct Node
{
	int data;
	int x;
	int y;
	Node* left;
	Node* right;

	Node(int val) : data(val), x(0), y(0), left(nullptr), right(nullptr)
	{
	}
};

extern Node* g_root;

Node* Insert(Node* node, int data);

Node* FindMin(Node* node);

Node* Delete(Node* node, int data);

void DeleteTree(Node* node);

void CalculatePositions(Node* node, int depth, int& orderIndex);

void DrawTree(HDC hdc, Node* node);
