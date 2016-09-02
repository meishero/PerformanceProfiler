#include"PerformanceProfiler.h"

void SingleThreadFun()
{
	PPsection* p1 = PerformanceProfiler::GetInstance()->CreateSection(__FILE__, __FUNCTION__, __LINE__, "网络");
	p1->Begin(-1);
	Sleep(1000);
	p1->End(-1);

	PPsection* p2 = PerformanceProfiler::GetInstance()->CreateSection(__FILE__, __FUNCTION__, __LINE__, "zhongjian");
	p2->Begin(-1);
	Sleep(2000);
	p2->End(-1);
}
void MultiThreadFun(size_t n)
{
	while (n--)
	{
		PPsection* p1 = PerformanceProfiler::GetInstance()->CreateSection(__FILE__, __FUNCTION__, __LINE__, "网络");
		p1->Begin(GetCurrentThreadId());
		Sleep(100);
		p1->End(GetCurrentThreadId());

		PPsection* p2 = PerformanceProfiler::GetInstance()->CreateSection(__FILE__, __FUNCTION__, __LINE__, "zhongjian");
		p2->Begin(GetCurrentThreadId());
		Sleep(200);
		p2->End(GetCurrentThreadId());

		
	}
}

void TestThread()
{
	//单线程
	//SingleThreadFun();
	
	
	//多线程
	thread t1(MultiThreadFun, 5);
	thread t2(MultiThreadFun, 4);
	thread t3(MultiThreadFun, 3);

	t1.join();
	t2.join();
	t3.join();

	//PerformanceProfiler::GetInstance()->Output();
}

//size_t fib(int n)
//{
//	PPsection* p = PerformanceProfiler::GetInstance()->CreateSection(__FILE__, __FUNCTION__, __LINE__, "递归");
//	p->Begin();
//	int ret = 0;
//	if (n <= 2)
//		ret = 1;
//	else
//		ret = fib(n - 1) + fib(n - 2);
//	
//	p->End();
//	
//	return ret;
//}
int main()
{
	//int a = 1;
	//char* p = (char*) &a;
	//if (*p == 1)
	//	printf("Little_endian");   //Little_endian
	//else
	//	printf("Big_endian");   //Big_endian
	//return 0;
	 //PerformanceProfiler test;


	/*PPsection* p = PerformanceProfiler::GetInstance()->CreateSection(__FILE__, __FUNCTION__, __LINE__, "网络");
	 p->Begin();
	 Sleep(1000);
	 p->End();
	 PerformanceProfiler::GetInstance()->Output();*/
		
	 //测试递归问题
	//fib(20);
	//PerformanceProfiler::GetInstance()->Output();//RAII

	

	TestThread();
	
}