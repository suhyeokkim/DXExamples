/*
	CPPAllocator : test of customized allocator in STL, EASTL

	this example indicate that customized allocator is implemented by overloaded new operator,
	and if not use system allocator, must implement custom allocator & delallocate in custom allocator.

	eastl container only call copy ctor.(only test eastl::vector) and not call move ctor. 
	this behavior indicate that 
	only way to implemetation of customized allocation is exploiting overloaded new/delete operator.
*/

#include <iostream>
#include <memory>
#include <vector>

// https://www.youtube.com/watch?v=pP15kDeXJU0
template <class T>
class TrackingAllocatorSTL
{
public:

#pragma region requirements

	using value_type = T;
	using pointer = T * ;
	using const_pointer = const T *;

	using size_type = size_t;

	TrackingAllocatorSTL() = default;
	template<class U>
	TrackingAllocatorSTL(const TrackingAllocatorSTL<U>& other) {};
	~TrackingAllocatorSTL() = default;

	pointer allocate(size_type numObjects)
	{
		alloccount += numObjects * sizeof(value_type);
		return static_cast<pointer>(operator new (sizeof(T) * numObjects));
	}

	void deallocate(pointer p, size_type numObjects)
	{
		alloccount -= numObjects * sizeof(value_type);
		operator delete(p);
	}
#pragma endregion

#pragma region optional
	template <class U>
	struct rebind
	{
		using other = TrackingAllocatorSTL<U>;
	};

	using void_pointer = void * ;
	using const_void_pointer = const void *;
	pointer allocate(size_type numObjects, const_void_pointer hint)
	{
		return allocate(numObjects);
	}

	size_type max_size() const
	{
		return std::numeric_limits<size_type>::max();
	}

	template<class U, class... Args>
	void construct(U *p, Args && ...args)
	{
		new (p) U(std::forward<Args>(args)...);
	}

	template<class U>
	void destroy(U *p)
	{
		p->~U();
	}
#pragma endregion

	size_type get_alloccount() const
	{
		return alloccount;
	}
	
private:
	static size_type alloccount;

};

template<class T>
typename TrackingAllocatorSTL<T>::size_type TrackingAllocatorSTL<T>::alloccount = 0;

// https://stackoverflow.com/questions/42565582/how-to-track-memory-usage-using-eastl

#include <EASTL/vector.h>
#include <EASTL/memory.h>

//void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
//{
//	return malloc(size);
//}
//
//void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
//{
//	return malloc(size);
//}

void operator delete(void* p)
{
	free(p);
}

void operator delete[](void* p)
{
	free(p);
}

class TrackingAllocatorEASTL
{
public:
	TrackingAllocatorEASTL(const char* name = EASTL_NAME_VAL(EASTL_ALLOCATOR_DEFAULT_NAME)) :
		name(nullptr), alloccount(0)
	{
		size_t len = strlen(name);
		this->name = (char*)malloc(sizeof(char) * (len + 1));
		strcpy_s(this->name, sizeof(char) * (len + 1), name);

		std::cout << this << ", " << name << ", init constructor call!" << std::endl;
	}

	explicit TrackingAllocatorEASTL(const TrackingAllocatorEASTL& o) : name(nullptr), alloccount(o.alloccount)
	{
		size_t len = strlen(o.name);
		name = (char*)malloc(sizeof(char) * (len+1));
		strcpy_s(name, sizeof(char) * (len + 1), o.name);

		std::cout << this << ", " << name << ", copy constructor call!" << std::endl;
	}
	explicit TrackingAllocatorEASTL(TrackingAllocatorEASTL&& o) : name(o.name), alloccount(o.alloccount)
	{
		std::cout << this << ", " << name << ", move constructor call!" << std::endl;
	}

	TrackingAllocatorEASTL& operator=(const TrackingAllocatorEASTL& o)
	{
		std::cout << this << ", " << name << ", copy operator= call!" << std::endl;

		size_t len = strlen(o.name);
		name = (char*)malloc(sizeof(char) * (len + 1));
		strcpy_s(name, sizeof(char) * (len + 1), o.name);

		alloccount = o.alloccount;
		return *this;
	}
	TrackingAllocatorEASTL& operator=(TrackingAllocatorEASTL&& o)
	{
		std::cout << this << ", " << name << ", move operator= call!" << std::endl;

		name = o.name;
		alloccount = o.alloccount;
		return *this;
	}

	void* allocate(size_t num_bytes, int flags = 0)
	{
		alloccount += num_bytes;
		void* p = ::new((char*)0, flags, 0, (char*)0, 0) char[num_bytes];
		return p;
	}

	void* allocate(size_t num_bytes, size_t alignment, size_t offset, int flags = 0)
	{
		alloccount += num_bytes;
		void* p = ::new(alignment, offset,(char*)0, flags, 0, (char*)0, 0) char[num_bytes];
		return p;
	}

	void deallocate(void* p, size_t num_bytes)
	{
		alloccount += num_bytes;
		delete[](char*)p;
	}

	const char* get_name() const { return name; }
	void set_name(char* n) { name = n; }
	size_t get_total_alloc_size() const { return alloccount; }

protected:
	char* name;
	size_t alloccount;

};

bool operator==(const TrackingAllocatorEASTL& a, const TrackingAllocatorEASTL& b)
{
	return !strcmp(a.get_name(), b.get_name());
}
bool operator!=(const TrackingAllocatorEASTL& a, const TrackingAllocatorEASTL& b)
{
	return strcmp(a.get_name(), b.get_name());
}

#include "../DX11SkinningWithFBX/allocators.h"

int main()
{
	using TAintSTL = TrackingAllocatorSTL<int>;
	using TAdoubleSTL = TrackingAllocatorSTL<int>::rebind<double>::other;

	std::vector<int, TAintSTL> v(5);

	std::cout << "v allocated size(before push) : " << v.get_allocator().get_alloccount() << std::endl;
	v.push_back(0);
	std::cout << "v allocated size(after push) : " << v.get_allocator().get_alloccount() << std::endl;

	{
		eastl::vector<int, EASTLAllocator> v2("temp");
		eastl::vector<int, EASTLAllocator> v3("temp");

		v2.reserve(32);
		v2.push_back(0);
		v3.reserve(32);
		v3.push_back(0);
	}
	{
		eastl::vector<int, EASTLAllocator> v2("temp");
		eastl::vector<int, EASTLAllocator> v3("temp");

		v2.reserve(64);
		v2.push_back(0);
		v3.reserve(64);
		v3.push_back(0);
	}
	{
		eastl::vector<int, EASTLAllocator> v2("temp");
		eastl::vector<int, EASTLAllocator> v3("temp");

		v2.reserve(128);
		v2.push_back(0);
		v3.reserve(128);
		v3.push_back(0);
	}
	{
		eastl::vector<int, EASTLAllocator> v2("temp");
		eastl::vector<int, EASTLAllocator> v3("temp");

		v2.reserve(256);
		v2.push_back(0);
		v3.reserve(256);
		v3.push_back(0);
	}

	return 0;
}