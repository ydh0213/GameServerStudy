#ifndef  __MEMORY_POOL__
#define  __MEMORY_POOL__

#include <new>
#include <cassert>
#include <cstddef>
#include <cstdint>

#ifdef _DEBUG
#if UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
constexpr uintptr_t SIGNATURE_ALLOCATED = 0xAAAAAAAAAAAAAAAA; // 64bit
#elif UINTPTR_MAX == 0xFFFFFFFF
constexpr uintptr_t SIGNATURE_ALLOCATED = 0xAAAAAAAA; // 32bit
#else
#error "지원하지 않는 포인터 크기 환경입니다."
#endif

constexpr unsigned int GUARD_VALUE = 0xDEADBEEF;
#endif

template <class DATA>
class MemoryPool
{
public:
	MemoryPool(int blockNum, bool placementNew = false) : m_capacity(0), m_useCount(0),
	                                                      m_placementNew(placementNew), _pFreeNode(nullptr)
	{
		// 요구한 개수만큼 노드를 만들어 FreeList(단일 연결 리스트)에 줄줄이 엮어둠
		for (int _ = 0; _ < blockNum; ++_)
		{
			st_BLOCK_NODE* pNewNode = new st_BLOCK_NODE;
			pNewNode->pNext = _pFreeNode;
			_pFreeNode = pNewNode;
			++m_capacity;
		}
	}

	virtual ~MemoryPool()
	{
		while (_pFreeNode != nullptr)
		{
			st_BLOCK_NODE* pDeleteNode = _pFreeNode;
			_pFreeNode = _pFreeNode->pNext;
			delete pDeleteNode;
		}
	}

	DATA* Alloc(void)
	{
		st_BLOCK_NODE* pNode = nullptr;

		// FreeList에 남은 노드가 있다면 맨 앞(Head / Top)에서 꺼내고, 없다면 새 노드를 동적 할당하여 풀의 크기를 늘림
		if (_pFreeNode != nullptr)
		{
			pNode = _pFreeNode;
			_pFreeNode = pNode->pNext;
		}
		else
		{
			pNode = new st_BLOCK_NODE;
			++m_capacity;
		}

		++m_useCount;

#ifdef _DEBUG
		pNode->frontGuard = GUARD_VALUE;
		pNode->rearGuard = GUARD_VALUE;
		pNode->pNext = reinterpret_cast<st_BLOCK_NODE*>(SIGNATURE_ALLOCATED);
#endif

		// 노드 내부의 char 배열 주소를 DATA 포인터로 형변환
		DATA* pData = reinterpret_cast<DATA*>(pNode->data);

		if (m_placementNew)
			new(pData) DATA;

		return pData;
	}

	bool Free(DATA* pData)
	{
		if (pData == nullptr)
			return false;

		st_BLOCK_NODE* pNode = reinterpret_cast<st_BLOCK_NODE*>(
			reinterpret_cast<char*>(pData) - offsetof(st_BLOCK_NODE, data)
		);

#ifdef _DEBUG
		assert(pNode->frontGuard == GUARD_VALUE && "Memory Underflow (앞쪽 메모리 침범) 감지!");
		assert(pNode->rearGuard == GUARD_VALUE && "Memory Overflow (뒤쪽 메모리 침범) 감지!");
		assert(pNode->pNext == reinterpret_cast<st_BLOCK_NODE*>(SIGNATURE_ALLOCATED)
			&& "이미 해제되었거나 pNext가 오염된 메모리입니다! (Double Free)");
#endif

		if (m_placementNew)
			pData->~DATA();

		// FreeList의 맨 앞(Head / Top)에 다시 끼워 넣기
		pNode->pNext = _pFreeNode;
		_pFreeNode = pNode;

		--m_useCount;
		return true;
	}

	int GetCapacityCount() { return m_capacity; }
	int GetUseCount() { return m_useCount; }

private:
	struct st_BLOCK_NODE
	{
#ifdef _DEBUG
		unsigned int frontGuard;
#endif

		// 객체를 미리 생성해두지 않고 Alloc 시점에 생성자를 호출하고 사용할 것이므로 DATA 타입의 메모리 정렬을 맞춰둠
		alignas(DATA) char data[sizeof(DATA)];

#ifdef _DEBUG
		unsigned int rearGuard;
#endif

		st_BLOCK_NODE* pNext;
	};

	int m_capacity;
	int m_useCount;
	bool m_placementNew;

	// 반환된 (미사용) 오브젝트 블럭들을 스택 방식으로 관리
	st_BLOCK_NODE* _pFreeNode;
};

#endif
