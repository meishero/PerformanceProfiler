#include"PerformanceProfiler.h"

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
	PPsection* p = PerformanceProfiler::GetInstance()->CreateSection(__FILE__, __FUNCTION__, __LINE__, "ÍøÂç");
	 p->Begin();
	 Sleep(1000);
	 p->End();
	 PerformanceProfiler::GetInstance()->Output();
}