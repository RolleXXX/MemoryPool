#include <iostream>
#include <cassert>
#include <time.h>
#include <vector>

#include "MemoryPool.h"
#include "StackAlloc.h"

/* Adjust these values depending on how much you trust your computer */

struct Point {
	int x;
	int y;
	Point(int _x = 0, int _y = 0) : x(_x), y(_y) {}

};
template< class T>
void create_Point(MemoryPool<T>& pool, std::vector<std::shared_ptr<T>> &points1,int id) {

	for (int i = 0; i < 1000000;i++) {
		auto p = pool.make_shared<T>(id,i, i);
		points1.emplace_back(p);
	}
	
	std::cout << "子线程" << id << "结束" << std::endl;


}

int main()
{

	
	//多线程
	MemoryPool<Point> pool;
	pool.addVec(1);
	pool.addVec(2);
	std::vector<std::shared_ptr<Point>> points1;
	std::vector<std::shared_ptr<Point>> points2;
	std::thread t1(create_Point<Point>, std::ref(pool), std::ref(points1),1);
	std::thread t2(create_Point<Point>, std::ref(pool), std::ref(points2),2);

	std::cout << t1.get_id() << std::endl;
	std::cout << t2.get_id() << std::endl;

	std::vector<std::shared_ptr<Point>> points;

	for (int i = 0; i < 1000000; i++) {
		auto p=pool.make_shared<Point>(0,i, i);
		points.emplace_back(p);
	}

	t1.join();
	t2.join();

	//测试多线程调用时有没有出现在同一块内存区域创建对象
	for (int i = 0; i < 1000000; i++) {
		if (points[i]->x != i || points[i]->y != i) {
			std::cout << i << std::endl;
			std::cout << points[i]->x << " " << points[i]->y << std::endl;
			break;
		}
	}
	

	//对比标准分配器，测试该内存池性能
	clock_t start;
	
	StackAlloc<int, MemoryAlloc<int> > stackPool;
	start = clock();
	for (int j = 0; j < 10; j++)
	{
		//assert(stackPool.empty());
		for (int i = 0; i < 1000000; i++) {
			// Unroll to time the actual code and not the loop
			stackPool.push(i);
			stackPool.push(i);
			stackPool.push(i);
			stackPool.push(i);
		}
		for (int i = 0; i < 1000000; i++) {
			// Unroll to time the actual code and not the loop
			stackPool.pop();
			stackPool.pop();
			stackPool.pop();
			stackPool.pop();
		}
	}
	std::cout << "MemoryPool Allocator Time: ";
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

	
	
	StackAlloc<int, std::allocator<int> > stackDefault;
	start = clock();
	for (int j = 0; j <10; j++)
	{
		for (int i = 0; i < 1000000; i++) {
			// Unroll to time the actual code and not the loop
			stackDefault.push(i);
			stackDefault.push(i);
			stackDefault.push(i);
			stackDefault.push(i);
		}
		for (int i = 0; i < 1000000; i++) {
			// Unroll to time the actual code and not the loop
			stackDefault.pop();
			stackDefault.pop();
			stackDefault.pop();
			stackDefault.pop();
		}
	}

	std::cout << "Default Allocator Time: ";
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

	return 0;
}