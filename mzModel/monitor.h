#pragma once
#include "mzModel.h"
#include "global.h"

using namespace std;

class MonitorPoint {
public:
	double X;
	double Y;
	int xIndex;
	int yIndex;
public:
	int init(double x, double y, double blx, double bly, int rows,int cols,double cellsize);
};

class Monitor {
public:
	int pointsCount=0;
	MonitorPoint* points=NULL;
	ifstream inputFile;
	ofstream outputFile;
public:
	int loadPoints(string path);
	int openPointsOut();
	int writePoints();
	int finalize();
};
