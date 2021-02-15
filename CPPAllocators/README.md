# CPPAllocator

STL/EASTL의 커스텀 할당자 구현 예제.

 - STL : 컴파일러 전처리에서 구현되는 할당자, 많은 코드 생성을 하게 되나 여러 것들을 상수로 놓음으로써 퍼포먼스를 높이기 위함이라고 함.
 - EASTL : 메모리에 상주하는 인스턴스 기반의 할당자, STL의 일반화된 정확성을 깨고 최적화를 위하여 인스턴스 기반의 구현으로 변경했다.

게임 프로그램에서는 최대한의 성능이 필요하기 때문에 유저레벨에서 페이징된 메모리 할당을 사용할 필요가 있다. 하지만 STL의 경우 게임의 잘 맞지 않고, 이를 게임에 맞도록 개선한 EASTL 을 사용하여 할당자를 구현하기로 했다.

# Implementation

커스텀 할당자를 구현한 사항은 다음과 같다.

 1. EASTL 사용시에 반드시 new 연산자 오버로딩이 필요하다. 이는 [해당 레포지토리 문서] (https://github.com/electronicarts/EASTL)에도 명시되어 있다. 이유는 디폴트 할당자가 이를 사용한다. 또한 커스텀 할당자 구현시에도 이를 활용하여 구현했다.
 2. delete 연산자 오버로딩을 페어로 해제를 구현하여 맞춰주었다. 다만 호출 시 덕지덕지 호출해야하여 연산자 오버로딩의 의미가 퇴색되지만 코드 상의 문맥을 맞춰주기 위하여 delete에 구현하였다. 이는 커스텀 할당자에서만 사용한다. 
 3. 오버로드된 new/delete 연산자 호출 시 OS 할당자에서 받은 페이지 내에서 공간을 배정해주고, 해제시 압축은 실행하지 않고 해당 데이터만 지운다. 
 4. 해제시 처리를 적게 하기 때문에 사용하는 메모리를 크게 persistant/temparary 로 나누어 사용한다.
 5. 컨테이너에서 사용되는 할당자는 어떠한 경우에도 최소한의 정렬된 메모리 할당을 구현해야 한다. 이는 매크로로 정의되어 있으며 작성 시점의 이름은 _EASTL\_ALLOCATOR\_MIN\_ALIGNMENT_ 이다.
 6. 할당/정렬된할당/해제 순서로 void\* allocate(size_t size, int flags), void\* allocate(size_t size, size_t align, size_t offset, int flags), void deallocate(void\* p, size_t num_bytes)을 구현해주어야 한다. 

# References

 - [EASTL FAQ: how do i assign a custom allocator to an eastl container](https://eastl.docsforge.com/master/faq/#cont8-how-do-i-assign-a-custom-allocator-to-an-eastl-container)
 - [open-std](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2271.html#better_allocator_model)
	
