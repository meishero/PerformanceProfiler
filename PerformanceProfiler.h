#pragma once
#include<iostream>
#include<map>
#include<windows.h>

#include<stdarg.h>
#include<ctime>

#include<stdio.h>
using namespace std;


static unsigned int BKDRhash(const char* key)
{//BKDRhash����
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

		string tmp2 = tmp._filename;
		tmp2 += tmp._line;
		tmp2 += tmp._function;
		return (BKDRhash(tmp1.c_str()) < BKDRhash(tmp1.c_str()));
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
	{}
	LongType Begin()
	{
		_beginTime = clock();
		return _beginTime;
	}
	LongType End()
	{
		_costTime = clock() - _beginTime;
		_callCount++;
		return _beginTime;		
	}
	LongType Count()
	{
		
		return _callCount;
		
	}

//protected:
	LongType _beginTime;
	LongType _callCount;
	LongType _costTime;
};



//template<class T>  //ΪPPNode ���Ƶ�less�º���
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


template<class T>   //��Ƶ����࣬����PerformanceProfiler
class Singleton
{
public:
	T* GetInstance()
	{
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

class PerformanceProfiler
{
public:
	typedef map<PPNode, PPsection*>  PPMAP;
	PerformanceProfiler()
	{}
	PPsection* CreateSection(const char* filename, const char* function,
		size_t line, const char* Desc)
	{
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
		PPMAP::iterator it = _ppmap.begin();
		while (it != _ppmap.end())
		{
			const PPNode& details = it->first;
			PPsection* section = it->second;
			sa.Save("Flie: %s, Line: %d, Function:%s, Desc:%s\n",
				details._filename.c_str(), details._line,\
				details._function.c_str(), details._Desc.c_str());
			sa.Save("BeginTime:%lld, CostTime:%lld��CallCount:%lld\n",
				section->Begin(), section->End(), section->Count());
			++it;
		}
		//sa.Save();
	}
protected:
	PPMAP _ppmap;  
};



