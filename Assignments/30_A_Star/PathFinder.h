#pragma once

struct CompareNode {
    bool operator()(const node* a, const node* b) const {
        if (a->f == b->f)
            return a->h > b->h;

        return a->f > b->f;
    }
};

class PathFinder
{
public:
    PathFinder();
    ~PathFinder();

    void Init();
    void ResetSearch();
    void ClearWalls();

    void SetStart(int x, int y);
    void SetEnd(int x, int y);
    void ToggleWall(int x, int y);

    // 한 단계 진행하고 계속 진행 가능한지 여부를 반환
    bool Step();

    node* GetNode(int x, int y) { return &grid[y][x]; }
    bool IsFound() const { return isFound; }
    bool IsSearching() const { return isSearching; }

private:
    node grid[HEIGHT][WIDTH];
    priority_queue<node*, vector<node*>, CompareNode> pq;

    node* startNode;
    node* endNode;
    node* currentNode;

    bool isSearching;
    bool isFound;
    bool isNoPath;

    int GetManhattan(int x1, int y1, int x2, int y2);
    int GetDistanceCost(int x1, int y1, int x2, int y2);
    void ReconstructPath() const;
};