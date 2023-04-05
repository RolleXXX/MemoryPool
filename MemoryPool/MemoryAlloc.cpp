#include "MemoryAlloc.h"


/*
�̰߳�ȫ���ڴ�أ�

*/


#ifndef MEMORY_BLOCK_CPP
#define MEMORY_BLOCK_CPP



template <typename T, size_t BlockSize>
inline typename MemoryAlloc<T, BlockSize>::size_type
MemoryAlloc<T, BlockSize>::padPointer(data_pointer_ p, size_type align)
const noexcept
{
	uintptr_t result = reinterpret_cast<uintptr_t>(p);
	//����������д���
	//return ((align - (result%align)) % align);
	return ((align - result) % align);
}



template <typename T, size_t BlockSize>
MemoryAlloc<T, BlockSize>::MemoryAlloc()
noexcept
{
	currentBlock_ = nullptr;
	currentSlot_ = nullptr;
	lastSlot_ = nullptr;
	freeSlots_ = nullptr;
	//is_mulThread = false;
}



template <typename T, size_t BlockSize>
MemoryAlloc<T, BlockSize>::MemoryAlloc(const MemoryAlloc& MemoryAlloc)
noexcept :
	MemoryAlloc()
{}



template <typename T, size_t BlockSize>
MemoryAlloc<T, BlockSize>::MemoryAlloc(MemoryAlloc&& MemoryAlloc)
noexcept
{
	currentBlock_ = MemoryAlloc.currentBlock_;
	MemoryAlloc.currentBlock_ = nullptr;
	currentSlot_ = MemoryAlloc.currentSlot_;
	lastSlot_ = MemoryAlloc.lastSlot_;
	freeSlots_ = MemoryAlloc.freeSlots_;
}


template <typename T, size_t BlockSize>
template<class U>
MemoryAlloc<T, BlockSize>::MemoryAlloc(const MemoryAlloc<U>& MemoryAlloc)
noexcept :
	MemoryAlloc()
{}



template <typename T, size_t BlockSize>
MemoryAlloc<T, BlockSize>&
MemoryAlloc<T, BlockSize>::operator=(MemoryAlloc&& MemoryAlloc)
noexcept
{
	if (this != &MemoryAlloc)
	{
		std::swap(currentBlock_, MemoryAlloc.currentBlock_);
		currentSlot_ = MemoryAlloc.currentSlot_;
		lastSlot_ = MemoryAlloc.lastSlot_;
		freeSlots_ = MemoryAlloc.freeSlots_;
	}
	return *this;
}



template <typename T, size_t BlockSize>
MemoryAlloc<T, BlockSize>::~MemoryAlloc()
noexcept
{
	slot_pointer_ curr = currentBlock_;
	while (curr != nullptr) {
		slot_pointer_ prev = curr->next;
		operator delete(reinterpret_cast<void*>(curr));
		curr = prev;
	}
}



template <typename T, size_t BlockSize>
inline typename MemoryAlloc<T, BlockSize>::pointer
MemoryAlloc<T, BlockSize>::address(reference x)
const noexcept
{
	return &x;
}



template <typename T, size_t BlockSize>
inline typename MemoryAlloc<T, BlockSize>::const_pointer
MemoryAlloc<T, BlockSize>::address(const_reference x)
const noexcept
{
	return &x;
}



template <typename T, size_t BlockSize>
void
MemoryAlloc<T, BlockSize>::allocateBlock()
{
	// Allocate space for the new block and store a pointer to the previous one
	data_pointer_ newBlock = reinterpret_cast<data_pointer_>
		(operator new(BlockSize));
	reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;
	currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);
	// Pad block body to staisfy the alignment requirements for elements
	data_pointer_ body = newBlock + sizeof(slot_pointer_);
	size_type bodyPadding = padPointer(body, alignof(slot_type_));
	currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);
	lastSlot_ = reinterpret_cast<slot_pointer_>
		(newBlock + BlockSize - sizeof(slot_type_) + 1);
}



template <typename T, size_t BlockSize>
inline typename MemoryAlloc<T, BlockSize>::pointer
MemoryAlloc<T, BlockSize>::allocate(size_type n, const_pointer hint)
{
	// ����п��еĶ���ۣ���ôֱ�ӽ��������򽻸���ȥ�����ѿ��ж���۵�ͷ�ڵ�����Ϊ��һ��
	{
		//m_mutex.lock();
		if (freeSlots_ != nullptr) {

			pointer result = reinterpret_cast<pointer>(freeSlots_);
			freeSlots_ = freeSlots_->next;
			//m_mutex.unlock();
			return result;
		}
		// �������۲������ˣ������һ���µ��ڴ�����
		else {
			if (currentSlot_ >= lastSlot_) {
				allocateBlock();
			}
				
			//m_mutex.unlock();
			return reinterpret_cast<pointer>(currentSlot_++);
		}
	}
	
}



template <typename T, size_t BlockSize>
inline void
MemoryAlloc<T, BlockSize>::deallocate(pointer p, size_type n)
{
	//�����ڴ��
	//std::unique_lock<std::mutex>lock(m_mutex);
	//m_mutex.lock();
	if (p != nullptr) {
		// reinterpret_cast ��ǿ������ת����
		reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
		freeSlots_ = reinterpret_cast<slot_pointer_>(p);
	}
	//m_mutex.unlock();
}


//
//template <typename T, size_t BlockSize>
//inline typename MemoryAlloc<T, BlockSize>::size_type
//MemoryAlloc<T, BlockSize>::max_size()
//const noexcept
//{
//	size_type maxBlocks = -1 / BlockSize;
//	return (BlockSize - sizeof(data_pointer_)) / sizeof(slot_type_) * maxBlocks;
//}


//Args&&... args ��һ���ɱ����ģ�������������������ʾһ����������Խ������������Ĳ���
//Args ��ģ�����������&&����ֵ�������η�����ʾ��Щ������������ֵ��������,���ⲻ��Ҫ�Ŀ���
template <typename T, size_t BlockSize>
template <class U, class... Args>
inline void
MemoryAlloc<T, BlockSize>::construct(U* p, Args&&... args)
{
	//����placement new��ָ���ڴ��ַ�Ϲ���һ��T
	//��������Ҫ�ֶ������ڴ������£���ʵ���Լ����ڴ�ء��ڴ��������
	//placement new �������ѷ���õ��ڴ�ռ��Ϲ�����󣬴Ӷ������˶�η�����ͷ��ڴ�Ŀ���
	new (p) U(std::forward<Args>(args)...);
	/*
	����ת���������������Ϊ��ֵ����ʱ������ֵ���ûᱨ��
	std::forward ���ݲ������������ж���ʹ����ֵ���û�����ֵ���ý���ת����
	�����������ֵ�������ͣ�����ת��Ϊ��ֵ�������ͣ������������ֵ�������ͣ�����ת��Ϊ��ֵ��������
	*/
}



template <typename T, size_t BlockSize>
template <class U>
inline void
MemoryAlloc<T, BlockSize>::destroy(U* p)
{
	p->~U();
}



template <typename T, size_t BlockSize>
template <class... Args>
inline typename MemoryAlloc<T, BlockSize>::pointer
MemoryAlloc<T, BlockSize>::newElement(Args&&... args)
{
	pointer result = allocate();
	construct<value_type>(result, std::forward<Args>(args)...);
	return result;
}

template<typename T, size_t BlockSize>
template<class U,class ...Args>
std::shared_ptr<T> MemoryAlloc<T, BlockSize>::make_shared(Args && ...args)
{
	//�̻߳�����ȫ

	
	/*std::cout << sizeof...(args) << std::endl;*/
	
	T* obj = allocate();
	
	construct(obj, std::forward<Args>(args)...);
	
	return std::shared_ptr<T>(obj, [this](T* ptr) {
		
		this->destroy(ptr);
		this->deallocate(ptr);
		
	});
}





template <typename T, size_t BlockSize>
inline void
MemoryAlloc<T, BlockSize>::deleteElement(pointer p)
{
	if (p != nullptr) {
		p->~value_type();
		deallocate(p);
	}
}



#endif // MEMORY_BLOCK_CPP