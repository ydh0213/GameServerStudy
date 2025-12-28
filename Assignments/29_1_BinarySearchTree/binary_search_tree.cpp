#include <windows.h>
#include <string>

#include "binary_search_tree.h"

// --- BST 로직: 삽입 ---
Node* Insert(Node* node, int data) {
    if (node == nullptr) return new Node(data);

    if (data < node->data)
        node->left = Insert(node->left, data);
    else if (data > node->data)
        node->right = Insert(node->right, data);

    return node;
}

// --- BST 로직: 최솟값 찾기 (삭제용) ---
Node* FindMin(Node* node) {
    while (node->left != nullptr) node = node->left;
    return node;
}

// --- BST 로직: 삭제 ---
Node* Delete(Node* node, int data) {
    if (node == nullptr) return node;

    if (data < node->data)
        node->left = Delete(node->left, data);
    else if (data > node->data)
        node->right = Delete(node->right, data);
    else {
        // 경우 1 & 2: 자식이 없거나 하나인 경우
        if (node->left == nullptr) {
            Node* temp = node->right;
            delete node;
            return temp;
        }
        else if (node->right == nullptr) {
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

// --- BST 로직: 트리 전체 삭제 ---
void DeleteTree(Node* node) {
    if (node == nullptr) return;
    DeleteTree(node->left);
    DeleteTree(node->right);
    delete node;
}

// --- 시각화 로직: 트리 그리기 (재귀) ---
// x, y: 현재 노드 위치, hGap: 가로 간격 (깊어질수록 좁아짐)
void DrawTree(HDC hdc, Node* node, int x, int y, int hGap) {
    if (node == nullptr) return;

    int radius = 15; // 노드 반지름
    int vGap = 60;   // 세로 간격

    // 1. 자식 노드로 잇는 선 그리기 (노드보다 뒤에 그려야 예쁨)
    if (node->left) {
        MoveToEx(hdc, x, y, NULL);
        LineTo(hdc, x - hGap, y + vGap);
        DrawTree(hdc, node->left, x - hGap, y + vGap, hGap / 2);
    }
    if (node->right) {
        MoveToEx(hdc, x, y, NULL);
        LineTo(hdc, x + hGap, y + vGap);
        DrawTree(hdc, node->right, x + hGap, y + vGap, hGap / 2);
    }

    // 2. 노드(원) 그리기
    HBRUSH brush = CreateSolidBrush(RGB(255, 255, 200)); // 연한 노란색
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    Ellipse(hdc, x - radius, y - radius, x + radius, y + radius);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);

    // 3. 값(텍스트) 그리기
    std::string text = std::to_string(node->data);
    SetBkMode(hdc, TRANSPARENT);
    SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
    TextOutA(hdc, x, y + 5, text.c_str(), (int)text.length());
}