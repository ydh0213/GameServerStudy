#pragma once

/// @brief 고정 크기 메모리 블록을 사용하는 원형 큐(Ring Buffer) 클래스
class RingBuffer
{
public:
	RingBuffer();

	/// @brief 지정된 용량으로 링버퍼를 생성합니다.
	/// @param capacity 버퍼 용량 (기본값: 4096)
	RingBuffer(int capacity = 1 << 12);

	~RingBuffer();

	// -----------------------------------------------------------
	// 상태 조회 함수
	// -----------------------------------------------------------

	int GetCapacity() const noexcept { return _capacity; }

	int GetUsedSize() const noexcept { return _size; }

	int GetFreeSize() const noexcept { return _capacity - _size; }

	// -----------------------------------------------------------
	// 입출력 함수
	// -----------------------------------------------------------

	/// @brief 데이터를 버퍼에 복사합니다.
	/// @param data 입력할 데이터 포인터
	/// @param size 입력할 데이터 크기
	/// @return (int) 실제로 입력된 데이터 크기 (공간 부족 시 잘림)
	int Enqueue(const char* data, int size);

	/// @brief 버퍼에서 데이터를 꺼내고 삭제합니다.
	/// @param dest 데이터를 저장할 목적지 포인터
	/// @param size 꺼낼 데이터 크기
	/// @return (int) 실제로 꺼낸 데이터 크기
	int Dequeue(char* dest, int size);

	/// @brief 데이터를 삭제하지 않고 확인만 합니다.
	/// @param dest 데이터를 저장할 목적지 포인터
	/// @param size 확인할 데이터 크기
	/// @return (int) 실제로 확인한 데이터 크기
	int Peek(char* dest, int size) const;

	/// @brief 버퍼를 초기화합니다. (메모리 해제는 하지 않음)
	void ClearBuffer();

	// -----------------------------------------------------------
	// 직접 접근(Direct Access) 지원 함수
	// -----------------------------------------------------------

	/// @brief 끊기지 않고 한 번에 쓸 수 있는 연속된 메모리 크기를 반환합니다.
	/// @details memcpy 등으로 데이터를 한 번에 밀어 넣을 때 유효한 길이를 알기 위해 사용합니다.
	/// @return (int) 연속된 쓰기 가능 용량
	int DirectEnqueueSize() const;

	/// @brief 끊기지 않고 한 번에 읽을 수 있는 연속된 메모리 크기를 반환합니다.
	/// @return (int) 연속된 읽기 가능 용량
	int DirectDequeueSize() const;

	/// @brief 쓰기 위치(Rear)를 강제로 이동시킵니다. (직접 쓰기 후 호출)
	/// @param size 이동할 크기
	/// @return (int) 실제 이동된 크기
	int MoveRear(int size);

	/// @brief 읽기 위치(Front)를 강제로 이동시킵니다. (직접 읽기 후 호출)
	/// @param size 이동할 크기
	/// @return (int) 실제 이동된 크기
	int MoveFront(int size);

	/// @brief 현재 읽기 위치(Front)의 메모리 포인터를 반환합니다.
	char* GetFrontBufferPtr() const;

	/// @brief 현재 쓰기 위치(Rear)의 메모리 포인터를 반환합니다.
	char* GetRearBufferPtr() const;

private:
	char* _buffer;
	int _capacity;
	int _size;
	int _frontPos;
	int _rearPos;
};