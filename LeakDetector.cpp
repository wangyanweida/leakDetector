#include <iostream>
#include <cstring>

#define __NEW_OVERLOAD_IMPLEMENTATION__
//这样避免了重载new运算符
//从而防止编译冲突如果重载,重载了new，和原来的不一样了。
#include "LeakDetector.hpp"

typedef struct _MemoryList {
	struct _MemoryList* next, *prev;
	size_t size;
	bool isArray;
	char* file;
	unsigned int line;
} _MemoryList;

static unsigned long _memory_allocated = 0;

static _MemoryList _root = {
	&_root, &_root, 0, false, NULL, 0
};

unsigned int _leak_detector::callCount = 0;

void* AllocateMemory(size_t _size, bool _array, char* _file, unsigned _line) {
	size_t newSize = sizeof(_MemoryList) + _size;
	_MemoryList* newElem = (_MemoryList*)malloc(newSize);

	newElem->next = _root.next;
	newElem->prev = &_root;
	newElem->size = _size;
	newElem->isArray= _array;
	newElem->file= NULL;

	if (_file) {
		newElem->file = (char*)malloc(strlen(_file) + 1);
		strcpy(newElem->file, _file);
	}

	newElem->line = _line;

	_root.next->prev = newElem;
	_root.next = newElem;

	_memory_allocated += _size;

	return (char*)newElem + sizeof(_MemoryList);
	//注意，这里强制转换为char*是因为严格控制指针每次+1只移动一个byte。
	//malloc只是返回一块内存的起始地址，p可以移动。释放动态内存时，必须让p指向malloc返回的指针，就是动态内存的起始位置。

}

void DeleteMemory(void* _ptr, bool _array) {
	//ptr指向的是申请内存的_size位置，跳过了MemoryList的长度。
	_MemoryList *currentElem = (_MemoryList*)((char*)_ptr-sizeof(_MemoryList));
	if(currentElem->isArray != _array) return;

	currentElem->prev->next = currentElem->next;
	currentElem->next->prev = currentElem->prev;
	_memory_allocated -= currentElem->size;
	free(currentElem);
}

void* operator new(size_t _size) {
	return AllocateMemory(_size, false, NULL, 0);
}

void* operator new[](size_t _size) {
	return AllocateMemory(_size, true, NULL, 0);
}

void* operator new(size_t _size, char* _file, unsigned int _line) {
	return AllocateMemory(_size, false, _file, _line);
}

void* operator new[](size_t _size, char* _file, unsigned int _line) {
	return AllocateMemory(_size, true, _file, _line);
}

void operator delete(void* _ptr) noexcept {
	DeleteMemory(_ptr, false);
}

void operator delete[](void* _ptr) noexcept {
	DeleteMemory(_ptr, true);
}

unsigned int _leak_detector::LeakDetector(void) noexcept {
	unsigned int count = 0;

	_MemoryList *ptr = _root.next;
	while (ptr && ptr != &_root) {
		if(ptr->isArray)
			std::cout << "leak[]";
		else
			std::cout << "leak";
		std::cout << ptr << " size " << ptr->size;
		if (ptr->file)
		  std::cout << "(locate in " << ptr->file << " line " 
			  << ptr->line <<")";
		else
			std::cout << "(Cannot find position)";

		std::cout << std::endl;
		++count;
		ptr = ptr->next;
	}

	if(count)
	  std::cout << "Total " << count << " leaks, size " 
		  << _memory_allocated << " byte. " << std::endl;
	return count;
}
