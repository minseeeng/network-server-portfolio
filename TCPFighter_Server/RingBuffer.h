#pragma once
#include <iostream>
class RingBuffer
{
public:
    RingBuffer();
    RingBuffer(int BufferSize);
    ~RingBuffer();

    void ReSize(int size);
    int GetBufferSize();

    int GetUseSize();
    int GetFreeSize();

    int Enqueue(const char* chpData, int size);
    int Dequeue(char* chpDest, int size);
    int Peek(char* chpDest, int size);

    void ClearBuffer();

    int DirectEnqueueSize();
    int DirectDequeueSize();

    int MoveRear(int Size);
    int MoveFront(int Size);

    char* GetFrontBufferPtr();
    char* GetRearBufferPtr();

private:
    char* readpointer;
    char* writepointer;
    // 데이터는 readpointer부터 writepointer 직전까지
    char* begin;
    char* end;
    int ringBufferSize;
};

