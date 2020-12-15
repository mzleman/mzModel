// mzModel.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <iomanip>
#include <time.h>
#include "couple.h"
#include <windows.h>

using namespace std;
int main(int argc, char *argv[])
{
	if (argc != 2) {
		cout << argc << endl;
		cout << "argc count wrong" << endl;
		return -1;
	}
	clock_t t1, t2;
	t1 = clock();
	//"E:\\Experiment\\20200612\\ModelData\\par.txt"
	modelInitialize(argv[1]);
	/*modelInitialize("E:\\Experiment\\20200722-mzModel\\ModelData\\demo.par");*/
	t2 = clock();
	cout << "\n初始化耗时:" << setprecision(4) << (double)(t2 - t1) / 1000.0 << "s" << endl;
	cout << setprecision(6) << endl;
	cout << "\n----------开始运行----------" << endl;
	t1 = clock();
	modelRun();
	cout << "\n----------运行结束----------" << endl;
	t2 = clock();
	cout << "\n运行耗时:" << setprecision(4) << (double)(t2 - t1) / 1000.0 << "s" << endl;
	if (parFile.swmm) {
		swmmEnd();
		swmmClose();
		FreeLibrary(SwmmDll);
	}
	final();
	return 0;
}


