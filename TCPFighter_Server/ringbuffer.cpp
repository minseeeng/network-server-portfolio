#include <iostream>
#include "RingBuffer.h"

RingBuffer::RingBuffer()
    : ringBufferSize(500)
{
    begin = (char*)malloc(ringBufferSize);
    end = begin + ringBufferSize;
    readpointer = writepointer = begin;
}

RingBuffer::RingBuffer(int BufferSize)
    :ringBufferSize(BufferSize)
{
    begin = (char*)malloc(ringBufferSize);
    end = begin + ringBufferSize;
    readpointer = writepointer = begin;
}

RingBuffer::~RingBuffer()
{
    free(begin);
}

void RingBuffer::ReSize(int size)
{

    int useSize = GetUseSize();
    char* newBuffer = (char*)malloc(size);
    char* newEnd = newBuffer + size;


    Peek(newBuffer, useSize);


    free(begin);

    begin = newBuffer;
    end = newEnd;
    ringBufferSize = size;
    readpointer = begin; // �����Ͱ� ������ ���ĵǾ����Ƿ� 0���� ����
    writepointer = begin + useSize;
}

int RingBuffer::GetBufferSize()
{
    return end - begin - 1;
}

int RingBuffer::GetUseSize()
{
    if (writepointer >= readpointer)
        return writepointer - readpointer;
    else
        return (writepointer - begin) + (end - readpointer);
}

int RingBuffer::GetFreeSize()
{
    if (writepointer >= readpointer)
        return (end - writepointer) + (readpointer - begin - 1);
    return readpointer - writepointer - 1;
}

int RingBuffer::DirectEnqueueSize()
{
    if (writepointer >= readpointer)
    {
        if (readpointer == begin)
            return (end - writepointer) - 1;
        else
            return end - writepointer;
    }
    else
    {
        return readpointer - writepointer - 1;
    }
}

int RingBuffer::DirectDequeueSize()
{
    if (writepointer >= readpointer)
        return writepointer - readpointer;
    return end - readpointer;
}

int RingBuffer::Enqueue(const char* chpData, int size)
{
    int freeSize = GetFreeSize();
    int enqueueSize;

    // if-else ������ ����Ͽ� ���� Enqueue�� ũ�� ����
    if (size < freeSize)
    {
        enqueueSize = size;
    }
    else
    {
        enqueueSize = freeSize;
    }

    if (enqueueSize == 0)
        return 0;

    int directEnqueue = DirectEnqueueSize();
    if (directEnqueue < enqueueSize)
    {
        memcpy_s(writepointer, directEnqueue, chpData, directEnqueue);
        int remainsize = enqueueSize - directEnqueue;
        memcpy_s(begin, remainsize, chpData + directEnqueue, remainsize);
    }
    else
    {
        memcpy_s(writepointer, enqueueSize, chpData, enqueueSize);
    }

    MoveRear(enqueueSize);
    return enqueueSize;
}

int RingBuffer::Dequeue(char* chpDest, int size)
{
    int useSize = GetUseSize();
    int dequeueSize;

    // if-else ������ ����Ͽ� ���� Dequeue�� ũ�� ����
    if (size < useSize)
    {
        dequeueSize = size;
    }
    else
    {
        dequeueSize = useSize;
    }

    if (dequeueSize == 0)
        return 0;

    if (DirectDequeueSize() < dequeueSize)
    {
        int directDequeuesize = DirectDequeueSize();
        memcpy_s(chpDest, directDequeuesize, readpointer, directDequeuesize);
        int remainsize = dequeueSize - directDequeuesize;
        memcpy_s(chpDest + directDequeuesize, remainsize, begin, remainsize);
    }
    else
    {
        memcpy_s(chpDest, dequeueSize, readpointer, dequeueSize);
    }

    MoveFront(dequeueSize);
    return dequeueSize;
}

int RingBuffer::Peek(char* chpDest, int size)
{
    int useSize = GetUseSize();
    int peekSize;

    // if-else ������ ����Ͽ� ���� Peek�� ũ�� ����
    if (size < useSize)
    {
        peekSize = size;
    }
    else
    {
        peekSize = useSize;
    }

    if (peekSize == 0)
        return 0;

    if (DirectDequeueSize() < peekSize)
    {
        int directDequeuesize = DirectDequeueSize();
        memcpy_s(chpDest, directDequeuesize, readpointer, directDequeuesize);
        int remainsize = peekSize - directDequeuesize;
        memcpy_s(chpDest + directDequeuesize, remainsize, begin, remainsize);
    }
    else
    {
        memcpy_s(chpDest, peekSize, readpointer, peekSize);
    }

    return peekSize;
}

int RingBuffer::MoveRear(int size)
{
    writepointer += size;
    if (writepointer >= end)
    {
        int overflow = writepointer - end;
        writepointer = begin + overflow;
    }
    return size;
}

int RingBuffer::MoveFront(int size)
{
    readpointer += size;
    if (readpointer >= end)
    {
        int overflow = readpointer - end;
        readpointer = begin + overflow;
    }
    return size;
}

void RingBuffer::ClearBuffer()
{
    readpointer = begin;
    writepointer = begin;
}

char* RingBuffer::GetFrontBufferPtr()
{
    return readpointer;
}

char* RingBuffer::GetRearBufferPtr()
{
    return writepointer;
}