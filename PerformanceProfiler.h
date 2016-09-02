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
	void Begin(int id)  //�����߳�ID
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
	//��ƶ��߳�ģʽ
	map<int, LongType> _beginTimeMap; //��ʽΪ:�̺߳ţ����ô���
	map<int, LongType> _callCountMap;
	map<int, LongType> _costTimeMap;
	map<int, LongType> _refCountMap;

	mutex _mtx;    

	//���߳�
	LongType _beginTime;
	LongType _callCount;
	LongType _costTime;
	LongType _refCount; //�������ü�������ֹ�ݹ�ͳ��_beginTime����������
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
	static T* GetInstance()
	{
		assert(_instance);  //����ģʽ  һ��Ҫȷ���˶�������
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
		lock_guard<mutex> lock(_mtx);//RAII ������

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
				section->_callCount += section->_callCountMap[timeIt->first];//�����߳�ID
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

			//ppNode��Ϣ
			sa.Save("Flie: %s, Line: %d, Function:%s, Desc:%s\n",
				details._filename.c_str(), details._line, \
				details._function.c_str(), details._Desc.c_str());

			//ppSection��Ϣ
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


			//����Ϣ
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
			sa.Save("BeginTime:%lld, CostTime:%lld��CallCount:%lld\n",
				section->Begin(), section->End(), section->Count());
			++it;*/
		
		//sa.Save();
	}
protected:
	PPMAP _ppmap;  
	mutex _mtx;
protected:
	

	//����Ϊ��Ԫ������ʵ��ʱ������Ҫ���ù��캯��
	friend class Singleton<PerformanceProfiler>; 

	PerformanceProfiler()
	{}
	PerformanceProfiler(const PerformanceProfiler&)
	{}
	PerformanceProfiler& operator=(const PerformanceProfiler&)
	{}
};

struct Report  //RAII ����output()����
{
	~Report()
	{
		PerformanceProfiler::GetInstance()->Output();
	}
};
static Report report;




