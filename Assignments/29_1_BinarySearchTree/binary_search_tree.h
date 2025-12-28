#pragma once

struct Node {
    int data;
    Node* left;
    Node* right;

    Node(int val) : data(val), left(nullptr), right(nullptr) {}
};

extern Node* g_root;

Node* Insert(Node* node, int data);

Node* FindMin(Node* node);

Node* Delete(Node* node, int data);

void DeleteTree(Node* node);

void DrawTree(HDC hdc, Node* node, int x, int y, int hGap);
