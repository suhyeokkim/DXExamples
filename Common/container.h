#pragma once

#include "symbols.h"
#include "defined_type.h"

#include <cstring>
#include <functional>

#define ARRAYLIST_DEFAULT_CAPACITY  32
#define ARRAYLIST_DEFAULT_ALIGNMENT 32

/// <summary>
/// C 스타일 컨테이너, ABI 를 위해 템플릿 사용 금지
/// </summary>
class DECLSPEC_DLL ArrayList
{
public: // crud
    bool Init(
        void (*initializer)(void*, void*), int32 step,
        int32 alignment = s_DefaultAlignment, uint64 capacity = s_DefaultCapacity
    );
    bool Destroy();
    bool CopyFrom(const ArrayList* list);
    bool CopyFrom(const ArrayList& list);

public: 
    bool ResizeMem(uint64 capacity);
    bool ResizeMore(uint64 count);

public:
    bool Insert(int32 startIndex, int32 count, NOTNULL void* ptr, NULLABLE void* param);
    bool Remove(int32 startIndex, int32 count);

public: // crud helper
    bool InsertStart(int32 count, NOTNULL void* ptr, NULLABLE void* param);
    bool InsertLast(int32 count, NOTNULL void* ptr, NULLABLE void* param);
    bool RemoveFirst();
    bool RemoveLast();
    bool RemoveAll();

public: // list accessor
    void* operator[](uint64 index) const;
    void* Start() const;
    void* Last() const;

    int32 Count() const;
    int32 Step() const;
    uint64 Capacity() const;
    int32 Alignment() const;

private:
    void* singleChunkPtr;
    void (*initializer)(void*, void*);

    int32 count;
    int32 step;
    uint64 capacity;
    int32 alignment;

public:
    static const uint64 s_DefaultCapacity  = ARRAYLIST_DEFAULT_CAPACITY;
    static const uint32 s_DefaultAlignment = ARRAYLIST_DEFAULT_ALIGNMENT;

};