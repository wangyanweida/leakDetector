## c++ 内存泄露检测器
[教程位置](https://www.shiyanlou.com/courses/657)
### 实现原理
1. 要实现内存泄露的检查，我们可以从下面几个点来思考：

	- 内存泄露产生于 new 操作进行后没有执行 delete
	- 最先被创建的对象，其析构函数永远是最后执行的

2. 重载new，使用一个双链表,记录下所有申请动态内存的位置。如果释放了动态内存，则删除对应的元素，最后根据双链表是否为空来判断内存泄露问题。

### 架构设计
1. 重载了new操作符，那么如果delete没有将申请的内存释放完毕，就一定发生了内存泄露。
2. 选择手动管理内存的数据结构：双链表
	- 不能提前确定什么时候申请动态内存，线性表不够合理
	- 删除内存检查器时，需要更新整个结构，单链表也不够好。
### 步骤
1. 定义宏：```#define new new(__FILE__, __LINE__)``` 来记录调用new的文件和行号

2. 定义一个结构体记录new申请的空间，结构体是双向链表。注意这个结构体申请的空间是结构体本身加上new 生成对象的长度，
	返回的指针是移动过的指针，指向new的对象空间，这里删除这个申请的空间时，记得要将指针转回去，free的指针必须是malloc返回的指针，
	因为malloc返回的指针前面几个字节的长度记录了malloc的长度信息，所以free时，也会取读取那几个字节的信息，如果指针不是malloc返回去的指针，
	则该指针前面没有长度元信息，会报段错误。

3. 重载new、new[]、delete、delete[]

### 总结
1. 重载new 和new[]操作符，需要加上noexcept,否则在这里抛出异常，会导致内存泄露问题。

2. 头文件中不能对类的非常量static赋值(如果是integer类型可以赋值，因为它相当与不变的对象)。因为头文件可能多次包含，而static在内存中的常量区，多次赋值可能导致内存模型不一致。

3. <font color=green>
	malloc只是返回一块内存的起始地址，p可以移动。释放动态内存时，
	必须让p指向malloc返回的指针，就是动态内存的起始位置。因为p的前面还有几个字节的申请空间长度信息。
</font>

4. new(a, b, c) T;
	会被解释成一个函数调用operator new(sizeof(T), a, b, c)。
	这是C++就有的行为 operator new, operator new[]，user-defined placement allocation functions。
	这里就是把那几个参数传给重载的operator new，这样可以在operator new里面记录下来 _NORMAL_BLOCK, __FILE__, __LINE__，调试的时候就能看到是在哪里new的。
