#ifndef MY_NEW_H
#define MY_NEW_H

#include <cstddef>

// =========================================================
// 오버로딩된 new / delete 선언부
// =========================================================
void* operator new(std::size_t size, const char* file, int line);
void* operator new[](std::size_t size, const char* file, int line);

void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;

// 예외 발생 시 짝을 맞추기 위한 delete (컴파일러 경고 방지)
void operator delete(void* ptr, const char* file, int line) noexcept;
void operator delete[](void* ptr, const char* file, int line) noexcept;

void operator delete(void* ptr, std::size_t size) noexcept;
void operator delete[](void* ptr, std::size_t size) noexcept;

// =========================================================
// 사용자 코드의 new를 가로채는 매크로 (STL 헤더 뒤에 와야 함)
// =========================================================
#define new new(__FILE__, __LINE__)

void DumpMemoryLeaks();

#endif // MY_NEW_H
