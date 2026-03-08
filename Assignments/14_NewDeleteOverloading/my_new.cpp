#include <unordered_map>
#include <mutex>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <new>

#include "my_new.h"

#undef new // 내부 로직에서는 우리가 만든 매크로 new를 사용하면 안 되므로 해제

// =========================================================
// 1. 커스텀 할당자 (Mallocator) - unordered_map 무한루프 방지
// =========================================================
template <typename T>
struct Mallocator
{
	using value_type = T;
	Mallocator() noexcept = default;

	template <typename U>
	Mallocator(const Mallocator<U>&) noexcept
	{
	}

	T* allocate(std::size_t n)
	{
		if (n > static_cast<std::size_t>(-1) / sizeof(T))
			throw std::bad_alloc();

		if (auto p = static_cast<T*>(std::malloc(n * sizeof(T))))
			return p;

		throw std::bad_alloc();
	}

	void deallocate(T* p, std::size_t) noexcept { std::free(p); }
};

template <typename T, typename U>
static bool operator==(const Mallocator<T>&, const Mallocator<U>&) noexcept { return true; }

template <typename T, typename U>
static bool operator!=(const Mallocator<T>&, const Mallocator<U>&) noexcept { return false; }

// =========================================================
// 2. 메모리 할당 정보 구조체
// =========================================================
struct AllocInfo
{
	void* ptr;
	std::size_t size;
	const char* filename;
	int line;
	bool array;
};

// =========================================================
// 3. 전역 메모리 관리 클래스 (싱글톤)
// =========================================================
class MemTrace
{
private:
	std::mutex mtx;
	char logFilename[128];
	bool isDestroyed = false;

	std::unordered_map<
		std::uintptr_t,
		AllocInfo,
		std::hash<std::uintptr_t>,
		std::equal_to<std::uintptr_t>,
		Mallocator<std::pair<const std::uintptr_t, AllocInfo>>
	> allocMap;

public:
	MemTrace()
	{
		time_t t = time(nullptr);
		tm tm_info;
		localtime_s(&tm_info, &t);
		snprintf(logFilename, sizeof(logFilename), "Alloc_%04d%02d%02d_%02d%02d%02d.txt",
		         tm_info.tm_year + 1900, tm_info.tm_mon + 1, tm_info.tm_mday,
		         tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec);
	}

	void DumpLeaks()
	{
		std::scoped_lock lock(mtx);

		for (const auto& [addr, info] : allocMap)
			if (info.filename && strcmp(info.filename, "Unknown (STL)") != 0)
				WriteLog("LEAK   ", addr, info.size, info.filename, info.line);

		allocMap.clear();
	}

	~MemTrace()
	{
		isDestroyed = true;
		DumpLeaks();
	}

	static MemTrace& GetInstance()
	{
		static MemTrace instance;
		return instance;
	}

	void Add(std::uintptr_t address, std::size_t size, const char* file, int line, bool isArray)
	{
		if (isDestroyed)
			return;

		std::scoped_lock lock(mtx);
		allocMap[address] = {(void*)address, size, file, line, isArray};
	}

	void Remove(std::uintptr_t address, bool isArray)
	{
		if (isDestroyed || address == 0)
			return;

		std::scoped_lock lock(mtx);

		auto it = allocMap.find(address);
		if (it != allocMap.end())
		{
			if (it->second.array != isArray)
				WriteLog("ARRAY  ", address, it->second.size, it->second.filename, it->second.line);

			allocMap.erase(it);
		}
		else
			WriteLog("NOALLOC", address, 0, nullptr, 0);
	}

private:
	void WriteLog(const char* type, std::uintptr_t address, std::size_t size, const char* file, int line)
	{
		FILE* fp = nullptr;
		fopen_s(&fp, logFilename, "a");

		if (!fp)
			return;

		if (file)
			fprintf(fp, "%-7s [%p] [%5zu] %s : %d\n", type, (void*)address, size, file, line);
		else
			fprintf(fp, "%-7s [%p]\n", type, (void*)address);

		fclose(fp);
	}
};

// =========================================================
// 4. 전역 new / delete 연산자 구현부
// =========================================================
void* operator new(std::size_t size, const char* file, int line)
{
	void* ptr = std::malloc(size);

	if (!ptr)
		throw std::bad_alloc();

	MemTrace::GetInstance().Add((std::uintptr_t)ptr, size, file, line, false);
	return ptr;
}

void* operator new[](std::size_t size, const char* file, int line)
{
	void* ptr = std::malloc(size);

	if (!ptr)
		throw std::bad_alloc();

	MemTrace::GetInstance().Add((std::uintptr_t)ptr, size, file, line, true);
	return ptr;
}

void* operator new(std::size_t size)
{
	void* ptr = std::malloc(size);

	if (!ptr)
		throw std::bad_alloc();

	MemTrace::GetInstance().Add((std::uintptr_t)ptr, size, "Unknown (STL)", 0, false);
	return ptr;
}

void* operator new[](std::size_t size)
{
	void* ptr = std::malloc(size);

	if (!ptr)
		throw std::bad_alloc();

	MemTrace::GetInstance().Add((std::uintptr_t)ptr, size, "Unknown (STL)", 0, true);
	return ptr;
}

void operator delete(void* ptr) noexcept
{
	MemTrace::GetInstance().Remove((std::uintptr_t)ptr, false);
	std::free(ptr);
}

void operator delete[](void* ptr) noexcept
{
	MemTrace::GetInstance().Remove((std::uintptr_t)ptr, true);
	std::free(ptr);
}

void operator delete(void* ptr, const char* file, int line) noexcept
{
	operator delete(ptr);
}

void operator delete[](void* ptr, const char* file, int line) noexcept
{
	operator delete[](ptr);
}

void operator delete(void* ptr, std::size_t size) noexcept
{
	operator delete(ptr);
}

void operator delete[](void* ptr, std::size_t size) noexcept
{
	operator delete[](ptr);
}

void DumpMemoryLeaks()
{
	MemTrace::GetInstance().DumpLeaks();
}