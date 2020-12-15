#include "mzModel.h"
#include "global.h"
#include "tools.h"
#include <string>
#include <cctype>
#include <algorithm>

using namespace std;

ParFile parFile;//global

int ParFile::init(string path) {
	ifstream parfile(path);
	if (!parfile.is_open()) {
		cout << "parfile path not exist:"<< path << endl;
		return -1;
	}
	string temp;
	int flag;
	while (!parfile.eof())
	{
		parfile >> temp;
		if (!temp.compare("nrows")) parfile >> this->modelRows;
		if (!temp.compare("ncols")) parfile >> this->modelCols;
		if (!temp.compare("xllcorner")) parfile >> this->modelBlx;
		if (!temp.compare("yllcorner")) parfile >> this->modelBly;
		if (!temp.compare("cellsize")) parfile >> this->modelDx;
		if (!temp.compare("NODATA_value")) parfile >> this->modelNullValue;
		if (!temp.compare("dirRoot")) parfile >> this->saveDir;
		if (!temp.compare("resultRoot")) parfile >> this->resultRoot;
		if (!temp.compare("sectionRoot")) parfile >> this->sectionRoot;
		if (!temp.compare("sectionCount")) parfile >> this->sectionCount;
		if (!temp.compare("simTime")) parfile >> this->simTime;
		if (!temp.compare("saveInterval")) parfile >> this->saveInterval;
		if (!temp.compare("initSaveTime")) parfile >> this->initSaveTime;
		if (!temp.compare("initTimeStep")) parfile >> this->initTimeStep;
		if (!temp.compare("uniformManning")) parfile >> this->uniformManning;
		//if (!temp.compare("radarRain")) this->radarRain = ON;
		//if (!temp.compare("rainGage")) this->rainGage = ON;
		if (!temp.compare("rainfallType")) {
			parfile >> temp;
			//cout << "降雨类型结果" << temp << endl;
			if (!temp.compare("RADAR")) this->radarRain = ON;
			if (!temp.compare("GAGE")) this->rainGage = ON;
			if (!temp.compare("SERIES"));
		}
		if (!temp.compare("rainfallFile")) { parfile >> this->rainfallFile; this->rainfall = ON; }
		if (!temp.compare("distributedEvap")) this->distributedEvap=ON;
		if (!temp.compare("evaporationFile")) {parfile >> this->evaporationFile; this->evaporation = ON; }
		if (!temp.compare("uniformInfil"))  {parfile >> this->uniformInfil; this->infiltration = ON; }
		if (!temp.compare("uniformLoss"))  {parfile >> this->uniformLoss;  this->earlyLoss = ON;}
		if (!temp.compare("routingSpeed")) parfile >> this->routingSpeed;
		if (!temp.compare("depthThresh")) parfile >> this->depthThresh;
		if (!temp.compare("theta")) parfile >> this->theta;
		if (!temp.compare("cfl")) parfile >> this->cfl;
		if (!temp.compare("g")) parfile >> this->g;
		if (!temp.compare("inpPath")) {
			parfile >> this->inpPath;
			this->swmm = ON;
		}
		

		if (!temp.compare("nodeReport")) 
		{
			this->reportNode = ON;
			parfile >> this->nodeReportPath;
		}
		

		if (!temp.compare("monitor")) 
		{ 
			parfile >> this->monitorPath; 
			this->monitor = ON; 
		}

		if (!temp.compare("uniformRules")) 
		{
			this->uniformRules = ON;
			parfile >> this->rulesPath;
		}

		if (!temp.compare("swmmSave")) 
		{
			parfile >> temp;
			this->swmmSave=flag=getflag(temp);
			cout << "swmmSave:" << flag << endl;
			if (flag == -1) return flag;
		}

		if (!temp.compare("maxH")) 
		{
			parfile >> temp;
			this->maxH = flag = getflag(temp);
			cout << "maxH:" << flag << endl;
			if (flag == -1) return flag;
		}

		if (!temp.compare("maxElev")) 
		{
			parfile >> temp;
			this->maxElev = flag = getflag(temp);
			cout << "maxElev:" << flag << endl;
			if (flag == -1) return flag;
		}

		if (!temp.compare("allowPonding"))
		{
			parfile >> temp;
			this->allowPonding = flag = getflag(temp);
			cout << "allowPonding:" << flag << endl;
			if (flag == -1) return flag;
		}

		if (!temp.compare("distributedManning")) 
		{
			parfile >> temp;
			this->distributedManning = flag = getflag(temp);
			cout << "distributedManning:" << flag << endl;
			if (flag == -1) return flag;
		}

		if (!temp.compare("distributedLoss")) 
		{
			this->earlyLoss = ON;
			parfile >> temp;
			this->distributedLoss = flag = getflag(temp);
			cout << "distributedLoss:" << flag << endl;
			if (flag == -1) return flag;
		}

		if (!temp.compare("distributedInfil")) 
		{
			this->infiltration = ON;
			parfile >> temp;
			this->distributedInfil = flag = getflag(temp);
			cout << "distributedInfil:" << flag << endl;
			if (flag == -1) return flag;
		}

		if (!temp.compare("acceleration")) 
		{
			parfile >> temp;
			this->acceleration = flag = getflag(temp);
			cout << "acceleration:" << flag << endl;
			if (flag == -1) return flag;
		}

		if (!temp.compare("routing")) 
		{
			parfile >> temp;
			this->routing = flag = getflag(temp);
			cout << "routing:" << flag << endl;
			if (flag == -1) return flag;
		}


	}
	cout << "\nParfile readed." << endl;
	return 0;
}