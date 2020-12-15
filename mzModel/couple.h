#pragma once
#include "global.h"

#define DW 4

using namespace std;

extern CouplePoint* QPoints;

//extern int NCouplePoint;  //2020-07-22取消该全局变量的使用 用CouplePoints::count替代

extern HINSTANCE SwmmDll;

typedef int(*SWMM_OPEN)(const char*, const char*, const char*);
extern SWMM_OPEN swmmOpen;

typedef int(*SWMM_START)(int);
extern SWMM_START swmmStart;

typedef int(*SWMM_END)(void);
extern SWMM_END swmmEnd;

typedef int(*SWMM_CLOSE)(void);
extern SWMM_CLOSE swmmClose;

typedef int(*SWMM_STEP)(double);
extern SWMM_STEP swmmStep;

typedef int(*GetCouplePointsN)(double blx, double bly, int row, int col, double delta);
extern GetCouplePointsN getCouplePointsN;

typedef int(*GetCouplePoints)(double blx, double bly, int row, int col, double delta, int* indexs, int* rows, int* cols, double* cqAs, double* weirBs,double* pondedA);
extern GetCouplePoints getCouplePoints;

typedef int(*GetSWMMTotalTime)(void);
extern GetSWMMTotalTime getSWMMTotalTime;

typedef double(*GetOverflow)(int);
extern GetOverflow getOverflow;

typedef int(*SetLatFlow)(int, double);
extern SetLatFlow setLatFlow;

typedef double(*GetSWMMTstep)(int, double);
extern GetSWMMTstep getSWMMTstep;

typedef int(*SetAllowPonding)(int);
extern SetAllowPonding setAllowPonding;

typedef double(*GetNodeHead)(int, int*);
extern GetNodeHead getNodeHead;

typedef void(*ReportNodeFlood)(const char*);
extern ReportNodeFlood reportNodeFlood;

int loadSWMM();
int initPoints();
void noPondingOverflow();
void pondingOverflow();