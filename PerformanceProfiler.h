#pragma once
#include<iostream>
#include<map>
#include<vector>
#include<windows.h>
#include<assert.h>
#include<mutex>
#include<thread>
#include<stdarg.h>
#include<ctime>

#include<stdio.h>
using namespace std;

typedef long long LongType;
static unsigned int BKDRhash(const char* key)
{//BKDRhash函数
	unsigned int seed = 131;
	unsigned int hash = 0;

	while (*key)
		hash = hash * seed + (*key++);

	return (hash & 0x7FFFFFFF);
}
struct PPNode
{
	string _function;
	size_t _line;
	string _filename;
	string _Desc;
	
	PPNode(const char* filename,const char* function, size_t line
		, const char* Desc)
			:_function(function)
			, _line(line)
			, _filename(filename)
			, _Desc(Desc)
	{}
	bool operator<(const PPNode& tmp) const
	{
		string tmp1 = _filename;
		tmp1 += _line;
		tmp1 += _function;
		tmp1 += _Desc;

		string tmp2 = tmp._filename;
		tmp2 += tmp._line;
		tmp2 += tmp._function;
		tmp2 += tmp._Desc;
		return (BKDRhash(tmp1.c_str()) < BKDRhash(tmp2.c_str()));
	}
};
class PPsection
{
public:

	typedef  long long LongType;
	PPsection()
		:_beginTime(0)
		, _costTime(0)
		, _callCount(0)
		, _refCount(0)
	{}
	void Begin(int id)  //传入线程ID
	{
		if (id != -1)
		{
			_mtx.lock();
			if (_refCountMap[id]++ == 0)
			{
				_beginTimeMap[id] = clock();
			}
			_mtx.unlock();
		}
		else 
		{
			if(_refCount++ == 0)
				_beginTime = clock();	
		}
		
		return;
	}
	void End(int id)
	{
		if (id != -1)
		{
			_mtx.lock();
			if (--_refCountMap[id] == 0)
			{
				_costTimeMap[id] += clock() - _beginTimeMap[id];	
			}
			++_callCountMap[id];
			_mtx.unlock();
		}
		else 
		{
			if(--_refCount == 0)
				_costTime += clock() - _beginTime;
			_callCount++;
		}
		
		return;
	}
	LongType Count()
	{
		return _callCount;	
	}

//protected:
//private:
	//设计多线程模式
	map<int, LongType> _beginTimeMap; //形式为:线程号，调用次数
	map<int, LongType> _callCountMap;
	map<int, LongType> _costTimeMap;
	map<int, LongType> _refCountMap;

	mutex _mtx;    

	//单线程
	LongType _beginTime;
	LongType _callCount;
	LongType _costTime;
	LongType _refCount; //设置引用计数，防止递归统计_beginTime的重置问题
};



//template<class T>  //为PPNode 定制的less仿函数
//struct lessPP
//{
//	bool operator()(const T&t1, const T& t2)
//	{
//		return t2 < t2;
//	}
//};
class SaveAdapter
{
public:
	bool virtual Save(const char* format, ...) const = 0;
};

class ConsoleSaveAdapter :public SaveAdapter
{
	bool Save(const char* format, ...) const
	{
		va_list args;
		va_start(args, format);
		vfprintf(stdout, format, args);
		va_end(args);
		return true;
	}
};


template<class T>   //设计单例类，用于PerformanceProfiler
class Singleton
{
public:
	static T* GetInstance()
	{
		assert(_instance);  //饿汉模式  一定要确保此对象生成
		return _instance;
	}
protected:
	static T* _instance;
protected:
	Singleton()
	{}
	Singleton(const T&)
	{}
	T& operator=(const T&)
	{}
};

template<class T>
T* Singleton<T>::_instance = new T;

class PerformanceProfiler :public Singleton<PerformanceProfiler>
{
public:
	typedef map<PPNode, PPsection*>  PPMAP;


	PPsection* CreateSection(const char* filename, const char* function,
		size_t line, const char* Desc)
	{
		lock_guard<mutex> lock(_mtx);//RAII 锁守卫

		PPNode ppnode(filename, function, line, Desc);
		PPsection* section = NULL;
		PPMAP::iterator it = _ppmap.find(ppnode);
		if (it != _ppmap.end())
		{
			section = it->second;
		}
		else
		{
			 section = new PPsection;
			_ppmap.insert(pair<PPNode, PPsection* >(ppnode, section));
		}
		return section;
	}
	void Output()
	{
		ConsoleSaveAdapter console;
		_Output(console);
	}
	void _Output(const SaveAdapter& sa)
	{
		vector<PPMAP::iterator> infos;

		PPMAP::iterator it = _ppmap.begin();
		while (it != _ppmap.end())
		{
			PPsection* section = it->second;
			map<int, LongType>::iterator timeIt = section->_costTimeMap.begin();
			while (timeIt != section->_costTimeMap.end())
			{
				section->_costTime += timeIt->second;
				section->_callCount += section->_callCountMap[timeIt->first];//利用线程ID
				timeIt++;
			}
			infos.push_back(it);
			it++;
		}
		for (int i = 0; i < infos.size(); i++)
		{
			it = infos[i];
			const PPNode& details = it->first;
			PPsection* section = it->second;

			//ppNode信息
			sa.Save("Flie: %s, Line: %d, Function:%s, Desc:%s\n",
				details._filename.c_str(), details._line, \
				details._function.c_str(), details._Desc.c_str());

			//ppSection信息
			map<int,LongType>::iterator timeIt = section->_costTimeMap.begin();

			while (timeIt != section->_costTimeMap.end())
			{
				int id = timeIt->first;
				sa.Save("ThreadId:%d, CostTime:%.2f s, CallCount:%lld\n"
					, id
					, (double)timeIt->second / 1000
					, section->_callCountMap[id]);

				section->_costTime += timeIt->second;
				section->_callCount += section->_callCountMap[id];

				++timeIt;
			}


			//总信息
			sa.Save("TotalCostTime:%.2f s, TotalCallCount:%lld, AverCostTime:%lld ms\n\n"
				, (double)section->_costTime / 1000
				, section->_callCount
				, section->_costTime / section->_callCount);
			
		}
		++it;

			/*const PPNode& details = it->first;
			PPsection* section = it->second;
			sa.Save("Flie: %s, Line: %d, Function:%s, Desc:%s\n",
				details._filename.c_str(), details._line,\
				details._function.c_str(), details._Desc.c_str());
			sa.Save("BeginTime:%lld, CostTime:%lld，CallCount:%lld\n",
				section->Begin(), section->End(), section->Count());
			++it;*/
		
		//sa.Save();
	}
protected:
	PPMAP _ppmap;  
	mutex _mtx;
protected:
	

	//声明为友元，创建实例时父类需要调用构造函数
	friend class Singleton<PerformanceProfiler>; 

	PerformanceProfiler()
	{}
	PerformanceProfiler(const PerformanceProfiler&)
	{}
	PerformanceProfiler& operator=(const PerformanceProfiler&)
	{}
};

struct Report  //RAII 进行output()操作
{
	~Report()
	{
		PerformanceProfiler::GetInstance()->Output();
	}
};
static Report report;




