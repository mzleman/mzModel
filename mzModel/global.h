#pragma once
#include "mzModel.h"
#include "monitor.h"
#include <string>
#include <Windows.h>

extern ParFile parFile;  //ParFile.cpp
extern Domain domain;  //Domain.cpp
extern QModel Qx;  //QModel.cpp
extern QModel Qy;  //QModel.cpp
extern Controls modelControl;  //Controls.cpp

extern Monitor ModelMonitor;
//extern ifstream MonitorFile;
//extern ofstream MonitorPointsOut;

int modelInitialize(string);
int modelRun();
int iterate();
int updateData();
int final();
void write_SpatialStructure(string,string,string,int,SpatialStructure &,int);
void saveData();







