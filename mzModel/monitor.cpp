
#include "global.h"
#include "files.h"
#include <iomanip>

using namespace std;

Monitor ModelMonitor;

//ifstream MonitorFile;
//ofstream MonitorPointsOut;

//===============================================================================================

int MonitorPoint::init(double x, double y, double blx, double bly,int rows,int cols, double cellsize) {
	this->X = x;
	this->Y = y;
	this->xIndex = (this->X - blx) / cellsize;
	this->yIndex = ((bly + rows * cellsize) - y) / cellsize;

	if (this->xIndex > cols) 
	{
		this->xIndex = -1;
	}
	if (this->yIndex >= rows) 
	{
		this->yIndex = -1;
	}

	//索引小于0则异常

	return 0;
}

//===============================================================================================
int Monitor::loadPoints(string path) {
	int count;
	double x, y;
	this->inputFile.open(path);
	if (!this->inputFile.is_open()) {
		cout << "Monitor File Open fail" << endl;
		return -1;
	}
	this->inputFile >> count;
	this->pointsCount = count;
	this->points = new MonitorPoint[count];
	for (int i = 0; i < count; i++) {
		this->inputFile >> x >> y;
		this->points[i].init( x, y, domain.blx, domain.bly, domain.rows, domain.cols, domain.dx);
	}
	this->inputFile.close();
	return 0;
}


//===============================================================================================

int Monitor::openPointsOut() 
{
	this->outputFile.open(parFile.saveDir + "\\" + parFile.resultRoot + ".mtr");
	if (!this->outputFile.is_open()) {
		cout << "monitor points output file open fail" << endl;
		return -1;
	}
	this->outputFile << "--------------------Points Depth Monitoring--------------------\n";
	cout << "monitoring points:" << this->pointsCount << endl;
	return 0;
}

//===============================================================================================

int Monitor::writePoints() //每隔saveInterval 输出所有监测点的水深
{
	int m, n;
	double* vals;

	if (!this->outputFile.is_open())
	{
		cout << "Open monitor output file fail" << endl;
		return -1;
	}

	this->outputFile << modelControl.saveTime << "s\t";
	for (int i = 0; i < this->pointsCount; i++) {
		m = points[i].yIndex;
		n = points[i].xIndex;

		if (m < 0 || n < 0) //点不在区域内
		{
			cout << "Monitor Points wrong:" << points[i].X << "\t" << points[i].Y << endl;
			continue;
		}

		vals = domain.waterDepths.cellArray[m][n].values;

		if (!vals) //该点没有水深值(没有dem)
		{
			cout << "Monitor Points wrong:" << points[i].X << "\t" << points[i].Y << endl;
			continue;
		}
		this->outputFile << setiosflags(ios::fixed)<<setprecision(4)<<vals[0]<<"\t";
	}
	this->outputFile << "\n";

	return 0;
}
//===============================================================================================


int Monitor::finalize() 
{
	if (this->points) 
	{
		delete[] this->points;
	}

	if (this->inputFile.is_open()) 
	{
		this->inputFile.close();
	}

	if (this->outputFile.is_open()) 
	{
		this->outputFile.close();
	}
	return 0;
}