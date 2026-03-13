#include <iostream>
#include <windows.h>
#include <conio.h>

using namespace std;

int main() {
    // 찾고자 하는 타겟 변수 (메모리 어딘가에 MEM_PRIVATE & MEM_COMMIT 상태로 할당됨)
    int hp = 1000;

    cout << "PID: " << GetCurrentProcessId() << std::endl;
    cout << "===============================" << std::endl;

    while (true) {
        cout << "\rHP: " << hp << "  (1: 증가, 2: 감소, q: 종료)   ";

        char ch = _getch();
        if (ch == '1')
            hp++;
        else if (ch == '2')
            hp--;
        else if (ch == 'q')
            break;
    }

    return 0;
}