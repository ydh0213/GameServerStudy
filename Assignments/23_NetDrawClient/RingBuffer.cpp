#include <algorithm>

#include "RingBuffer.h"

using namespace std;

RingBuffer::RingBuffer() = default;

RingBuffer::RingBuffer(int capacity) : _buffer(new char[capacity]), _capacity(capacity), _size(0), _frontPos(0),
_rearPos(0)
{
}

RingBuffer::~RingBuffer()
{
	delete[] _buffer;
}

int RingBuffer::Enqueue(const char* data, int size)
{
	size = min(size, GetFreeSize());

	if (size <= 0) return 0;

	const int firstChunk = min(size, DirectEnqueueSize());
	memcpy_s(GetRearBufferPtr(), DirectEnqueueSize(), data, firstChunk);

	const int secondChunk = size - firstChunk;
	if (secondChunk > 0)
		memcpy_s(_buffer, _capacity, data + firstChunk, secondChunk);

	MoveRear(size);

	return size;
}

int RingBuffer::Dequeue(char* dest, int size)
{
	size = min(size, GetUsedSize());

	if (size <= 0) return 0;

	const int firstChunk = min(size, DirectDequeueSize());
	memcpy_s(dest, size, GetFrontBufferPtr(), firstChunk);

	const int secondChunk = size - firstChunk;
	if (secondChunk > 0)
		memcpy_s(dest + firstChunk, secondChunk, _buffer, secondChunk);

	MoveFront(size);

	return size;
}

int RingBuffer::Peek(char* dest, int size) const
{
	size = min(size, GetUsedSize());

	if (size <= 0) return 0;

	const int firstChunk = min(size, DirectDequeueSize());
	memcpy_s(dest, size, GetFrontBufferPtr(), firstChunk);

	const int secondChunk = size - firstChunk;
	if (secondChunk > 0)
		memcpy_s(dest + firstChunk, secondChunk, _buffer, secondChunk);

	return size;
}

void RingBuffer::ClearBuffer()
{
	_size = 0;
	_frontPos = 0;
	_rearPos = 0;
}

int RingBuffer::DirectEnqueueSize() const
{
	if (_size == _capacity)
		return 0;

	if (_frontPos <= _rearPos)
		return _capacity - _rearPos;

	return _frontPos - _rearPos;
}

int RingBuffer::DirectDequeueSize() const
{
	if (_size == 0)
		return 0;

	if (_frontPos < _rearPos)
		return _rearPos - _frontPos;

	return _capacity - _frontPos;
}

int RingBuffer::MoveRear(int size)
{
	if (size <= 0)
		return 0;

	int realSize = min(size, GetFreeSize());

	_rearPos = (_rearPos + realSize) % _capacity;
	_size += realSize;

	return realSize;
}

int RingBuffer::MoveFront(int size)
{
	if (size <= 0)
		return 0;

	int realSize = min(size, GetUsedSize());

	_frontPos = (_frontPos + realSize) % _capacity;
	_size -= realSize;

	return realSize;
}

char* RingBuffer::GetFrontBufferPtr() const
{
	return _buffer + _frontPos;
}

char* RingBuffer::GetRearBufferPtr() const
{
	return _buffer + _rearPos;
}
