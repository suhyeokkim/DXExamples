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
		alloccount += numObjects;
		return static_cast<pointer>(operator new (sizeof(T) * numObjects));
	}

	void deallocate(pointer p, size_type numObejcts)
	{
		alloccount--;
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
		alloccount += numObjects;
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

// https://stackoverflow.com/questions/42565582/how-to-track-memory-usage-using-eastl

#include <EASTL/vector.h>
#include <EASTL/memory.h>

template<class T>
typename TrackingAllocatorSTL<T>::size_type TrackingAllocatorSTL<T>::alloccount = 0;

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return malloc(size);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return malloc(size);
}

class TrackingAllocatorEASTL
{
public:
	using size_type = size_t;

	TrackingAllocatorEASTL(const char* name = EASTL_NAME_VAL(EASTL_ALLOCATOR_DEFAULT_NAME)) :
		name(name), alloccount(0)
	{
	}

	TrackingAllocatorEASTL(const TrackingAllocatorEASTL& o) : name(o.name), alloccount(o.alloccount) { }
	TrackingAllocatorEASTL(const TrackingAllocatorEASTL& o, const char* name) : 
		name(name), alloccount(o.alloccount) 
	{
	}

	TrackingAllocatorEASTL& operator=(const TrackingAllocatorEASTL& o)
	{
		name = o.name;
		alloccount = o.alloccount;
		return *this;
	}

	void* allocate(size_type num_bytes, int flags = 0)
	{
		alloccount += num_bytes;
		void* p = ::new((char*)0, flags, 0, (char*)0, 0) char[num_bytes];
		return p;
	}

	void* allocate(size_type num_bytes, size_type alignment, size_t offset, int flags = 0)
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
	void set_name(const char* n) { name = n; }
	size_t get_total_alloc_size() const { return alloccount; }

protected:
	const char* name;
	size_type alloccount;

};

bool operator==(const TrackingAllocatorEASTL& a, const TrackingAllocatorEASTL& b)
{
	return &a == &b;
}
bool operator!=(const TrackingAllocatorEASTL& a, const TrackingAllocatorEASTL& b)
{
	return &a != &b;
}

int main()
{
	using TAintSTL = TrackingAllocatorSTL<int>;
	using TAdoubleSTL = TrackingAllocatorSTL<int>::rebind<double>::other;

	std::vector<int, TAintSTL> v(5);

	std::cout << v.get_allocator().get_alloccount() << std::endl;
	v.push_back(0);
	std::cout << v.get_allocator().get_alloccount() << std::endl;

	eastl::vector<int, TrackingAllocatorEASTL> v2(5);

	std::cout << v2.get_allocator().get_total_alloc_size() << std::endl;
	v2.push_back(0);
	std::cout << v2.get_allocator().get_total_alloc_size() << std::endl;

	return 0;
}