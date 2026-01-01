#pragma once

constexpr int RADIUS = 15;
constexpr int HOR_GAP = 8;
constexpr int VER_GAP = 80;

enum NODE_COLOR
{
	BLACK = 0,
	RED
};

struct Node
{
	int data;
	int x;
	int y;
	NODE_COLOR color;
	Node* parent;
	Node* left;
	Node* right;

	Node(int val, NODE_COLOR c, Node* p, Node* l, Node* r) :
		data(val), x(0), y(0), color(c), parent(p), left(l), right(r)
	{
	}
};

extern Node nil;
extern Node* g_root;

void LeftRotate(Node* x);

void RightRotate(Node* y);

void _InsertFixUp(Node* z);

void Insert(int val);

Node* _FindMin(Node* z);

Node* Search(int val);

// u 위치에 v를 이식
void RB_Transplant(Node* u, Node* v);

void _DeleteFixUp(Node* x);

void Delete(int val);

void DeleteTree(Node* node);

void CalculatePositions(Node* node, int depth, int& orderIndex);

void DrawTree(HDC hdc, Node* node);

// 블랙 높이 반환, 에러 발생 시 -1 반환
int ValidateHelper(Node* node, string& errorMsg);

bool ValidateTree(Node* root, string& errorMsg);
