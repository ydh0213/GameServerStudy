#include <windows.h>
#include <string>

using namespace std;

#include "red_black_tree.h"

void LeftRotate(Node* x)
{
	Node* y = x->right;
	x->right = y->left;
	y->left->parent = x;

	y->parent = x->parent;

	if (x == g_root)
		g_root = y;
	else if (x == x->parent->left)
		x->parent->left = y;
	else
		x->parent->right = y;

	y->left = x;
	x->parent = y;
}

void RightRotate(Node* y)
{
	Node* x = y->left;
	y->left = x->right;
	x->right->parent = y;

	x->parent = y->parent;

	if (y == g_root)
		g_root = x;
	else if (y == y->parent->left)
		y->parent->left = x;
	else
		y->parent->right = x;

	x->right = y;
	y->parent = x;
}

void _InsertFixUp(Node* z)
{
	while (z->parent->color == RED)
	{
		Node* p = z->parent;
		Node* g = p->parent;

		// 내가 할아버지의 왼자손
		if (p == g->left)
		{
			Node* u = g->right;

			// Case 1. 부모, 삼촌이 다 RED: 할아버지 BLACK을 양 자식에게 내리고 node를 할아버지로 이동
			if (u->color == RED)
			{
				p->color = BLACK;
				u->color = BLACK;
				g->color = RED;
				z = g;
			}
			else
			{
				// Case 2. 내가 부모의 오른자식: 부모 기준으로 left rotate로 Case 3로 만든다
				if (p->right == z)
				{
					z = p;
					LeftRotate(z);
					p = z->parent;
					g = p->parent;
				}

				// Case 3. 내가 부모의 왼자식: 색 바꾸면서 할아버지 기준으로 right rotate
				p->color = BLACK;
				g->color = RED;
				RightRotate(g);
			}
		}
		else // 내가 할아버지의 오른자손
		{
			Node* u = g->left;

			// Case 1. 부모, 삼촌이 다 RED: 할아버지 BLACK을 양 자식에게 내리고 node를 할아버지로 이동
			if (u->color == RED)
			{
				p->color = BLACK;
				u->color = BLACK;
				g->color = RED;
				z = g;
			}
			else
			{
				// Case 2. 내가 부모의 왼자식: 부모 기준으로 right rotate로 Case 3로 만든다
				if (z == p->left)
				{
					z = p;
					RightRotate(z);
					p = z->parent;
					g = p->parent;
				}

				// Case 3. 내가 부모의 오른자식: 색 바꾸면서 할아버지 기준으로 left rotate
				p->color = BLACK;
				g->color = RED;
				LeftRotate(g);
			}
		}
	}

	g_root->color = BLACK;
}

void Insert(int val)
{
	if (g_root == nullptr || g_root == &nil)
	{
		g_root = new Node(val, BLACK, &nil, &nil, &nil);
		return;
	}

	Node* y = g_root;

	while (true)
		if (val < y->data)
		{
			if (y->left == &nil)
			{
				_InsertFixUp(y->left = new Node(val, RED, y, &nil, &nil));
				break;
			}

			y = y->left;
		}
		else if (y->data < val)
		{
			if (y->right == &nil)
			{
				_InsertFixUp(y->right = new Node(val, RED, y, &nil, &nil));
				break;
			}

			y = y->right;
		}
		else // 이미 있는 값이면 무시
			break;
}

Node* _FindMin(Node* z)
{
	while (z != nullptr && z != &nil && z->left != &nil)
		z = z->left;

	return z;
}

Node* Search(int val)
{
	Node* z = g_root;

	while (z && z != &nil)
		if (val < z->data)
			z = z->left;
		else if (z->data < val)
			z = z->right;
		else
			break;

	return z;
}

void RB_Transplant(Node* u, Node* v)
{
	if (u->parent == &nil)
		g_root = v;
	else if (u == u->parent->left)
		u->parent->left = v;
	else
		u->parent->right = v;

	v->parent = u->parent;
}

void _DeleteFixUp(Node* x)
{
	while (x != g_root && x->color == BLACK)
	{
		Node* p = x->parent;

		if (x == p->left)
		{
			Node* w = p->right; // 형제

			// Case 1. 형제가 RED: 형제-부모 색상 바꾸고 left rotate
			if (w->color == RED)
			{
				w->color = BLACK;
				p->color = RED;
				LeftRotate(p);
				w = p->right;
			}

			Node* l = w->left;
			Node* r = w->right;

			// Case 2. 형제의 두 자식 모두 BLACK: 형제만 RED로 만들고 x를 위로 올림
			if (l->color == BLACK && r->color == BLACK)
			{
				w->color = RED;
				x = p;
			}
			else
			{
				// Case 3. 형제의 왼자식 RED, 오른자식 BLACK: 형제를 right rotate
				if (r->color == BLACK)
				{
					l->color = BLACK;
					w->color = RED;
					RightRotate(w);
					w = p->right;
					r = w->right;
				}

				// Case 4. 형제의 오른자식 RED: 색상 교환, left rotate
				w->color = p->color;
				p->color = BLACK;
				r->color = BLACK;
				LeftRotate(p);
				x = g_root;
			}
		}
		else
		{
			Node* w = p->left; // 형제

			// Case 1. 형제가 RED: 형제-부모 색상 바꾸고 right rotate
			if (w->color == RED)
			{
				w->color = BLACK;
				p->color = RED;
				RightRotate(p);
				w = p->left;
			}

			Node* l = w->left;
			Node* r = w->right;

			// Case 2. 형제의 두 자식 모두 BLACK: 형제만 RED로 만들고 x를 위로 올림
			if (l->color == BLACK && r->color == BLACK)
			{
				w->color = RED;
				x = p;
			}
			else
			{
				// Case 3. 형제의 왼자식 BLACK, 오른자식 RED: 형제를 left rotate
				if (l->color == BLACK)
				{
					r->color = BLACK;
					w->color = RED;
					LeftRotate(w);
					w = p->left;
					l = w->left;
				}

				// Case 4. 형제의 왼자식 RED: 색상 교환, right rotate
				w->color = p->color;
				p->color = BLACK;
				l->color = BLACK;
				RightRotate(p);
				x = g_root;
			}
		}
	}

	x->color = BLACK;
}

void Delete(int val)
{
	Node* z = Search(val);

	if (z == &nil || z == nullptr)
		return;

	Node* y = z;
	Node* x;
	NODE_COLOR yColor = y->color;

	// 자식이 하나 이하인 경우
	if (z->left == &nil)
	{
		x = z->right;
		RB_Transplant(z, z->right);
	}
	else if (z->right == &nil)
	{
		x = z->left;
		RB_Transplant(z, z->left);
	}
	// 자식이 둘인 경우
	else
	{
		y = _FindMin(z->right);
		yColor = y->color;
		x = y->right;

		if (y->parent == z)
			x->parent = y; // x가 NIL이어도 일단 부모를 y로
		else
		{
			RB_Transplant(y, x);
			y->right = z->right;
			y->right->parent = y;
		}

		RB_Transplant(z, y);
		y->left = z->left;
		y->left->parent = y;
		y->color = z->color;
	}

	delete z;

	if (yColor == BLACK)
		_DeleteFixUp(x);
}

void DeleteTree(Node* node)
{
	if (node == nullptr || node == &nil)
		return;

	DeleteTree(node->left);
	DeleteTree(node->right);
	delete node;
}

void CalculatePositions(Node* node, int depth, int& orderIndex)
{
	if (node == nullptr) return;

	CalculatePositions(node->left, depth + 1, orderIndex);

	node->x = orderIndex * HOR_GAP + 20;
	node->y = depth * VER_GAP + 80;

	++orderIndex;

	CalculatePositions(node->right, depth + 1, orderIndex);
}

void DrawTree(HDC hdc, Node* node)
{
	if (node == nullptr || node == &nil) return;

	if (node->left != &nil)
	{
		MoveToEx(hdc, node->x, node->y, NULL);
		LineTo(hdc, node->left->x, node->left->y);
		DrawTree(hdc, node->left);
	}
	if (node->right != &nil)
	{
		MoveToEx(hdc, node->x, node->y, NULL);
		LineTo(hdc, node->right->x, node->right->y);
		DrawTree(hdc, node->right);
	}

	HBRUSH brush;
	COLORREF textColor;

	if (node->color == RED)
	{
		brush = CreateSolidBrush(RGB(255, 80, 80));
		textColor = RGB(0, 0, 0);
	}
	else
	{
		brush = CreateSolidBrush(RGB(60, 60, 60));
		textColor = RGB(255, 255, 255);
	}

	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);

	Ellipse(hdc, node->x - RADIUS, node->y - RADIUS, node->x + RADIUS, node->y + RADIUS);

	SelectObject(hdc, oldBrush);
	DeleteObject(brush);

	string text = to_string(node->data);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, textColor);
	SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
	TextOutA(hdc, node->x, node->y + 5, text.c_str(), text.length());

	SetTextColor(hdc, RGB(0, 0, 0));
}

int ValidateHelper(Node* node, string& errorMsg)
{
	if (node == &nil)
	{
		if (node->color != BLACK)
		{
			errorMsg = "[Violation] Nil 노드는 BLACK 이어야 합니다.";
			return -1;
		}

		return 1;
	}

	if (node->color == RED && (node->left->color == RED || node->right->color == RED))
	{
		errorMsg = "[Violation] 연속 2개 Red 노드: " + to_string(node->data);
		return -1;
	}

	if (node->left != &nil && node->left->data >= node->data)
	{
		errorMsg = "[BST Violation] 왼쪽 자식 노드 " + to_string(node->left->data) + " >= 부모 노드 " + to_string(node->data);
		return -1;
	}

	if (node->right != &nil && node->data >= node->right->data)
	{
		errorMsg = "[BST Violation] 부모 노드 " + to_string(node->data) + " >= 오른쪽 자식 노드 " + to_string(node->right->data);
		return -1;
	}

	int leftBH = ValidateHelper(node->left, errorMsg);
	if (leftBH == -1)
		return -1;

	int rightBH = ValidateHelper(node->right, errorMsg);
	if (rightBH == -1)
		return -1;

	if (leftBH != rightBH)
	{
		errorMsg = "[Violation] Black 높이가 다른 노드: " + to_string(node->data) + " (L:" + to_string(leftBH) + ", R:" +
			to_string(rightBH) + ")";
		return -1;
	}

	return leftBH + (node->color == BLACK ? 1 : 0);
}

bool ValidateTree(Node* root, string& errorMsg)
{
	if (root == &nil)
		return true;

	if (root->color != BLACK)
	{
		errorMsg = "[Violation] Root 노드는 BLACK 이어야 합니다.";
		return false;
	}

	return ValidateHelper(root, errorMsg) != -1;
}
