#include <windows.h>
#include <string>

#include "binary_search_tree.h"

// --- 삽입 ---
Node* Insert(Node* node, int data)
{
	if (node == nullptr) return new Node(data);

	if (data < node->data)
		node->left = Insert(node->left, data);
	else if (data > node->data)
		node->right = Insert(node->right, data);

	return node;
}

// --- 최솟값 찾기 (삭제용) ---
Node* FindMin(Node* node)
{
	while (node->left != nullptr) node = node->left;
	return node;
}

// --- 삭제 ---
Node* Delete(Node* node, int data)
{
	if (node == nullptr) return node;

	if (data < node->data)
		node->left = Delete(node->left, data);
	else if (data > node->data)
		node->right = Delete(node->right, data);
	else
	{
		// 경우 1 & 2: 자식이 없거나 하나인 경우
		if (node->left == nullptr)
		{
			Node* temp = node->right;
			delete node;
			return temp;
		}

		if (node->right == nullptr)
		{
			Node* temp = node->left;
			delete node;
			return temp;
		}

		// 경우 3: 자식이 둘인 경우
		Node* temp = FindMin(node->right);
		node->data = temp->data;
		node->right = Delete(node->right, temp->data);
	}

	return node;
}

// --- 트리 전체 삭제 ---
void DeleteTree(Node* node)
{
	if (node == nullptr) return;

	DeleteTree(node->left);
	DeleteTree(node->right);
	delete node;
}

// --- 좌표 계산 로직 (중위 순회) ---
// orderIndex: 현재 몇 번째 노드인지 세는 변수 (참조로 전달하여 계속 증가시킴)
void CalculatePositions(Node* node, int depth, int& orderIndex)
{
	if (node == nullptr) return;

	// 1. 왼쪽 자식 먼저 방문
	CalculatePositions(node->left, depth + 1, orderIndex);

	// 2. 나(Root)의 위치 결정
	// X좌표: 순서 * 간격
	// Y좌표: 깊이 * 간격 + 상단여백(80px)
	node->x = orderIndex * HOR_GAP + 20;
	node->y = depth * VER_GAP + 80;

	++orderIndex; // 순서 카운트 증가

	// 3. 오른쪽 자식 방문
	CalculatePositions(node->right, depth + 1, orderIndex);
}

// --- 그리기 로직 ---
void DrawTree(HDC hdc, Node* node)
{
	if (node == nullptr) return;

	if (node->left)
	{
		MoveToEx(hdc, node->x, node->y, NULL);
		LineTo(hdc, node->left->x, node->left->y);
		DrawTree(hdc, node->left);
	}
	if (node->right)
	{
		MoveToEx(hdc, node->x, node->y, NULL);
		LineTo(hdc, node->right->x, node->right->y);
		DrawTree(hdc, node->right);
	}

	HBRUSH brush = CreateSolidBrush(RGB(220, 230, 255)); // 연한 파랑
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);

	Ellipse(hdc, node->x - RADIUS, node->y - RADIUS, node->x + RADIUS, node->y + RADIUS);

	SelectObject(hdc, oldBrush);
	DeleteObject(brush);

	std::string text = std::to_string(node->data);
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
	TextOutA(hdc, node->x, node->y + 5, text.c_str(), (int)text.length());
}
