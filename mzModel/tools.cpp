#include "tools.h"
#include <direct.h>
#include <io.h>
#include <iostream>
using namespace std;

void sortArr_A(double *arr, int length) {
	bool didSwap;
	double temp;
	for (int i = 0, len = length; i < len - 1; i++) {
		didSwap = false;
		for (int j = 0; j < len - i - 1; j++) {
			if (arr[j + 1] < arr[j]) {
				temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
				didSwap = true;
			}
		}
		if (didSwap == false)
			return;
	}
}

void sortArr_D(double *arr, int length) {
	bool didSwap;
	double temp;
	for (int i = 0, len = length; i < len - 1; i++) {
		didSwap = false;
		for (int j = 0; j < len - i - 1; j++) {
			if (arr[j + 1] > arr[j]) {
				temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
				didSwap = true;
			}
		}
		if (didSwap == false)
			return;
	}
}

double arrMaxdouble(double arr[], int len) {
	double max = arr[0];
	for (int i = 1; i < len; i++)
		if (max < arr[i]) max = arr[i];
	return max;
}

double maxDouble(double a1, double a2) {
	return a1 > a2 ? a1 : a2;
}
double minDouble(double a1, double a2) {
	return a1 < a2 ? a1 : a2;
}
int findIndexInArray(string s, string* arr, int len) {
	for (int i = 0; i < len; i++) {
		if (!arr[i].compare(s)) return i;
	}
	return -1;
}
bool makeDir(string path) {
	string command;
	if (0 != _access(path.c_str(), 0)) //判断目录是否已经存在  _access()函数存在于<io.h>中
	{
		/*command = "mkdir -p " + path;
		system(command.c_str());*/
		if (_mkdir(path.c_str())) { cout << "CREATE DIR FAIL:" << path << endl; }
		return true;
	}
	return false;
}

void trimDirPath(string &path) {
	int end = path.find_last_not_of("\\");
	path = path.substr(0, end + 1); //清除路径右侧的反斜杠
}
//for (int i = 1; i <= parFile.sectionCount; i++) {
//	string sectionDirPath = parFile.saveDir + "\\section#" + std::to_string(i);
//	if (0 != _access(sectionDirPath.c_str(), 0))
//	{
//		cout << "新建目录" << sectionDirPath << endl;
//		command = "mkdir " + sectionDirPath;
//		system(command.c_str());
//	}
//
//}
//return 0;

int getflag(string flag) {
	if (!flag.compare("ON") || !flag.compare("YES")) return 1;
	else if (!flag.compare("OFF") || !flag.compare("NO")) return 0;
	else return -1;
}