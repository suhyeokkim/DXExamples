#include <string.h>

#include "defined_macro.h"
#include "defined_alloc_macro.h"
#include "allocators.h"

bool operator==(const EASTLAllocator& a, const EASTLAllocator& b)
{
	return !strcmp(a.get_name(), b.get_name());
}
bool operator!=(const EASTLAllocator& a, const EASTLAllocator& b)
{
	return strcmp(a.get_name(), b.get_name());
}

void* SystemAlloc(size_t size, unsigned debugFlags, const char* file, int line)
{
	if (!debugFlags)
		return malloc(size);
	else
		return _malloc_dbg(size, _CLIENT_BLOCK, file, line);
}
void* SystemAlignedAlloc(size_t size, size_t alignment, size_t alignmentOffset, unsigned debugFlags, const char* file, int line)
{
	if (!debugFlags)
		return _aligned_offset_malloc(size, alignment, alignmentOffset);
	else
		return _aligned_offset_malloc_dbg(size, alignment, alignmentOffset, file, line);
}
void SystemDealloc(void* p, unsigned debugFlags, const char* file, int line)
{
	if (!debugFlags)
		free(p);
	else
		_free_dbg(p, _CLIENT_BLOCK);
}
void SystemAlignedDealloc(void* p, unsigned debugFlags, const char* file, int line)
{
	if (!debugFlags)
		_aligned_free(p);
	else
		_aligned_free_dbg(p);
}

struct MemRange
{
	size_t start, count;
	int flags;
	MemRange() : start(0), count(0), flags(0) {}
	MemRange(size_t start, size_t count, int flags) : start(start), count(count), flags(flags) {}
};
#include <iostream>
struct MemChunk
{
	void* mem_ptr;
	size_t size;
	eastl::vector<MemRange, EASTLAllocator> ranges;

	MemChunk() : mem_ptr(nullptr), size(0), ranges(EASTL_SYSTEM_NAME) {}
	MemChunk(void* mem_ptr, size_t size) : mem_ptr(mem_ptr), size(size), ranges(EASTL_SYSTEM_NAME)
	{
		ranges.set_capacity(size / 1024 + 1);
	}
	MemChunk(const MemChunk& o) = default;
	MemChunk(MemChunk&& o) = default;
	MemChunk& operator=(const MemChunk&) = default;
	MemChunk& operator=(MemChunk&&) = default;

#define CEIL_ALIGNED_TO(addr, align, offset) ((((addr) + (align - 1)) & ~(align - 1)) + offset)
#define FLOOR_ALIGNED_TO(addr, align, offset) (((addr) & ~(align - 1)) + offset)

	bool GetEmptyMemAndAppend(IN size_t req_size, IN size_t alignment, IN size_t aligned_offset, IN int flags, OUT void** ptr)
	{
		size_t start_point = eastl::numeric_limits<size_t>::max();

		if (ranges.size() > 0)
		{
			size_t aligned_start = 0, aligned_end = 0, insert_index = 0;

			for (size_t i = 0; i < ranges.size() + 1; i++)
			{
				if (i == 0)
				{
					aligned_start = 0;
					aligned_end = FLOOR_ALIGNED_TO(ranges[0].start, alignment, aligned_offset);
				}
				else if (i == ranges.size())
				{
					auto it = eastl::prev(ranges.end());
					aligned_start = CEIL_ALIGNED_TO(it->start + it->count, alignment, aligned_offset);
					aligned_end = this->size;
				}
				else
				{
					aligned_start = CEIL_ALIGNED_TO(ranges[i - 1].start + ranges[i - 1].count, alignment, aligned_offset);
					aligned_end = FLOOR_ALIGNED_TO(ranges[i].start, alignment, aligned_offset);
				}

				if (req_size + aligned_start < aligned_end)
				{
					start_point = aligned_start;
					insert_index = i;
					break;
				}
			}

			if (start_point == eastl::numeric_limits<size_t>::max())
				return false;

			ranges.insert(ranges.begin() + insert_index, MemRange(start_point, req_size, flags));
		}
		else
		{
			start_point = 0;
			ranges.push_back(MemRange(start_point, req_size, flags));
		}

		*ptr = static_cast<void*>(((char*)mem_ptr + start_point));

		return true;
	}

	bool RemoveRange(IN void* p, OUT size_t* count)
	{
		if (mem_ptr <= p && (void*)((char*)mem_ptr + size) > p)
		{
			decltype(ranges.begin()) rit, rend;
			for (rit = ranges.begin(), rend = ranges.end(); rit != rend; rit++)
				if ((void*)((char*)mem_ptr + rit->start) == p)
					break;

			if (rit != rend)
			{
				ranges.erase(rit);
				if (count) *count = rit->count;
				return true;
			}
		}

		return false;
	}
};

struct AllocatorEntry
{
	char* name;
	unsigned debug;
	size_t alloc_byte_count;
	size_t min_page_size;
	size_t last_ref_page;
	eastl::vector<MemChunk, EASTLAllocator> mem_chunks;

	const size_t name_buffer_max = 256;

	AllocatorEntry() :
		name(nullptr), mem_chunks(EASTL_SYSTEM_NAME), min_page_size(0), last_ref_page(0), debug(0), alloc_byte_count(0)
	{
	}
	AllocatorEntry(const char* name, unsigned debug, size_t min_page_size) :
		mem_chunks(EASTL_SYSTEM_NAME), min_page_size(min_page_size), last_ref_page(0), debug(debug), alloc_byte_count(0)
	{
		ALLOC_SIZE_AND_STRCPY(nullptr, this->name, name_buffer_max, name);
	}
	AllocatorEntry(const AllocatorEntry& o) :
		mem_chunks(o.mem_chunks, EASTL_SYSTEM_NAME), min_page_size(o.min_page_size), last_ref_page(o.last_ref_page), debug(o.debug), alloc_byte_count(o.alloc_byte_count)
	{
		ALLOC_SIZE_AND_STRCPY(nullptr, this->name, name_buffer_max, name);
		for (auto it = mem_chunks.begin(), end = mem_chunks.end(); it != end; it++)
			it->mem_ptr = SystemAlignedAlloc(it->size, min_page_size, 0, debug, __FILE__, __LINE__); 
	}
	AllocatorEntry(const AllocatorEntry& o, const char* name) :
		mem_chunks(o.mem_chunks, EASTL_SYSTEM_NAME), min_page_size(o.min_page_size), last_ref_page(o.last_ref_page), debug(o.debug), alloc_byte_count(o.alloc_byte_count)
	{
		ALLOC_SIZE_AND_STRCPY(nullptr, this->name, name_buffer_max, name);
		for (auto it = mem_chunks.begin(), end = mem_chunks.end(); it != end; it++)
			it->mem_ptr = SystemAlignedAlloc(it->size, min_page_size, 0, debug, __FILE__, __LINE__);
	}
	~AllocatorEntry()
	{
		if (!name) return;

		for (auto it = mem_chunks.begin(), end = mem_chunks.end(); it != end; it++)
			if (it->mem_ptr)
				SystemAlignedDealloc(it->mem_ptr, debug, __FILE__, __LINE__);
	}

	void* allocate(size_t num_bytes, int flags = 0)
	{
		return allocate(num_bytes, EASTL_ALLOCATOR_MIN_ALIGNMENT, 0, flags);
	}
	void* allocate(size_t num_bytes, size_t alignment, size_t offset, int flags = 0)
	{
		alloc_byte_count += num_bytes;

		if (mem_chunks.size() == 0)
		{
			size_t alloc_size = num_bytes < min_page_size ? min_page_size : num_bytes;
			void* page = SystemAlignedAlloc(alloc_size, min_page_size, 0, debug, __FILE__, __LINE__);
			mem_chunks.push_back(MemChunk(page, alloc_size));
			last_ref_page = 0;
		}

		void *p = nullptr;
		if (mem_chunks[last_ref_page].GetEmptyMemAndAppend(num_bytes, alignment, offset, flags, &p))
			return p;

		for (auto it = mem_chunks.rbegin(), end = mem_chunks.rend(); it != end; it)
			if (it->GetEmptyMemAndAppend(num_bytes, alignment, offset, flags, &p))
			{
				last_ref_page = mem_chunks.size() - eastl::distance(mem_chunks.rbegin(), it) - 1;
				return p;
			}

		size_t alloc_size = num_bytes < min_page_size ? min_page_size : num_bytes;
		void* page = SystemAlignedAlloc(alloc_size, min_page_size, 0, debug, __FILE__, __LINE__);
		mem_chunks.push_back(MemChunk(page, alloc_size));
		last_ref_page = mem_chunks.size() - 1;

		if (mem_chunks[last_ref_page].GetEmptyMemAndAppend(num_bytes, alignment, offset, flags, &p))
			return p;
		else
			return nullptr;
	}

	void deallocate(void* p, size_t num_bytes)
	{
		for (auto it = mem_chunks.begin(), end = mem_chunks.end(); it != end; it++)
		{
			size_t count;
			if (it->RemoveRange(p, &count))
			{
				alloc_byte_count -= count;
				break;
			}
		}
	}
};

const int AllocEntryCount = 16;
AllocatorEntry g_Entries[AllocEntryCount];

size_t AddEntry(const char* name, size_t min_page_size)
{
	size_t index = eastl::numeric_limits<size_t>::max();
	for (size_t i = 0; i < AllocEntryCount; i++)
		if (g_Entries[i].name == nullptr)
		{
			index = i;
			break;
		}

	if (index == eastl::numeric_limits<size_t>::max()) return index;

	g_Entries[index].~AllocatorEntry();
	new (g_Entries + index) AllocatorEntry(name, 1, min_page_size);

	return index;
}
AllocatorEntry* FindEntry(const char* name)
{
	for (size_t i = 0; i < AllocEntryCount; i++)
		if (g_Entries[i].name != nullptr && !strcmp(g_Entries[i].name, name))
			return g_Entries + i;
	return nullptr;
}

const size_t MinPageSize = 16 * 1024 * 1024;

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return ::new (EASTL_ALLOCATOR_MIN_ALIGNMENT, 0, pName, flags, debugFlags, file, line) char[size];
}
void* operator new[](size_t size, const char* pName, const char* file, int line)
{
	unsigned dbg = 
#if defined DEBUG | _DEBUG
		1
#else
		0
#endif
		;

	return ::new (EASTL_ALLOCATOR_MIN_ALIGNMENT, 0, pName, 0, dbg, file, line) char[size];
}
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	if (pName == nullptr || pName[0] == '\0')
		return SystemAlignedAlloc(size, alignment, alignmentOffset, debugFlags, file, line);

	AllocatorEntry* entry = FindEntry(pName);

	if (entry == nullptr)
	{
		size_t index = AddEntry(pName, MinPageSize);

		if (index < 0)
		{
			ASSERT(L"fail to allocate page..");
			return nullptr;
		}

		entry = g_Entries + index;
	}

	return entry->allocate(size, alignment, alignmentOffset, flags);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, const char* file, int line)
{
	unsigned dbg =
#if defined DEBUG | _DEBUG
		1
#else
		0
#endif
		;

	return ::new (alignment, alignmentOffset, pName, 0, dbg, file, line) char[size];
}

void operator delete[](void* p, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	if (pName == nullptr || pName[0] == '\0')
	{
		SystemAlignedDealloc(p, debugFlags, file, line);
		return;
	}

	AllocatorEntry* entry = FindEntry(pName);

	if (entry == nullptr)
	{
		size_t index = AddEntry(pName, MinPageSize);

		if (index < 0)
		{
			ASSERT(L"fail to allocate page..");
			return;
		}

		entry = g_Entries + index;
	}

	entry->deallocate(p, 0);
}

void operator delete[](void* p, const char* pName, const char* file, int line)
{
	unsigned dbg =
#if defined DEBUG | _DEBUG
		1
#else
		0
#endif
		;

	::operator delete[] (p, pName, 0, dbg, file, line);
}
void operator delete[](void* p, unsigned align, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	if (pName == nullptr || pName[0] == '\0')
	{
		SystemDealloc(p, debugFlags, file, line);
		return;
	}

	AllocatorEntry* entry = FindEntry(pName);

	if (entry == nullptr)
	{
		size_t index = AddEntry(pName, MinPageSize);

		if (index < 0)
		{
			ASSERT(L"fail to allocate page..");
			return;
		}

		entry = g_Entries + index;
	}

	entry->deallocate(p, 0);
}
void operator delete[](void* p, unsigned align, const char* pName, const char* file, int line)
{
	unsigned dbg =
#if defined DEBUG | _DEBUG
		1
#else
		0
#endif
		;

	::operator delete[](p, align, pName, 0, dbg, file, line);
}
