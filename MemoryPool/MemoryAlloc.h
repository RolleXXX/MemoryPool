#pragma once
/*
���ڴ��ʵ����make_shared���������ɹ���ָ���������ڴ����
�����ĳЩС����������ҪƵ�������ڴ棬�뿪�������Զ��ͷ�

ʵ���̰߳�ȫ��ÿ���������ڴ�������ڴ���ʱ��Ҫ��ʱ������Ƶ�������ͷ�������Ч�ʵ���
��������ʱ�������ͷ�ʱ��ʱ�ͷš���һ����ʱ����ȥ�����ڴ���й����ڴ�����������һ���ͷŶ���ڴ��

�̰߳�ȫ�ڶ��ַ���:ÿһ���߳��ڹ̶��������Ͻ����ڴ����

���õڶ���
*/

#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <climits>
#include <cstddef>
#include <memory>
#include <iostream>
#include <mutex>




template <typename T, size_t BlockSize = 4096>
class MemoryAlloc
{
public:
	/* Member types */
	typedef T               value_type;
	typedef T*              pointer;
	typedef T&              reference;
	typedef const T*        const_pointer;
	typedef const T&        const_reference;
	typedef size_t          size_type;
	typedef ptrdiff_t       difference_type;
	typedef std::false_type propagate_on_container_copy_assignment;
	typedef std::true_type  propagate_on_container_move_assignment;
	typedef std::true_type  propagate_on_container_swap;

	template <typename U> struct rebind {
		typedef MemoryAlloc<U> other;
	};

	/* Member functions */
	MemoryAlloc() noexcept;
	MemoryAlloc(const MemoryAlloc& MemoryAlloc) noexcept;
	MemoryAlloc(MemoryAlloc&& MemoryAlloc) noexcept;
	template <class U> MemoryAlloc(const MemoryAlloc<U>& MemoryAlloc) noexcept;

	~MemoryAlloc() noexcept;

	MemoryAlloc& operator=(const MemoryAlloc& MemoryAlloc) = delete;
	MemoryAlloc& operator=(MemoryAlloc&& MemoryAlloc) noexcept;

	pointer address(reference x) const noexcept;
	const_pointer address(const_reference x) const noexcept;

	// Can only allocate one object at a time. n and hint are ignored
	pointer allocate(size_type n = 1, const_pointer hint = 0);
	void deallocate(pointer p, size_type n = 1);

	//size_type max_size() const noexcept;

	template <class U, class... Args> void construct(U* p, Args&&... args);
	template <class U> void destroy(U* p);

	template <class... Args> pointer newElement(Args&&... args);
	void deleteElement(pointer p);

	template <class U, class... Args>
	std::shared_ptr<T> make_shared(Args&&... args);



private:
	union Slot_ {
		value_type element;
		Slot_* next;
	};

	typedef char* data_pointer_;
	typedef Slot_ slot_type_;
	typedef Slot_* slot_pointer_;
	
	slot_pointer_ currentBlock_;
	slot_pointer_ currentSlot_;
	slot_pointer_ lastSlot_;
	slot_pointer_ freeSlots_;

	

	size_type padPointer(data_pointer_ p, size_type align) const noexcept;
	void allocateBlock();

	static_assert(BlockSize >= 2 * sizeof(slot_type_), "BlockSize too small.");
};


#include "MemoryAlloc.cpp"

#endif // MEMORY_POOL_H

