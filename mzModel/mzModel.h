#pragma once


#include <iostream>
#include <time.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <omp.h>

#define ON 1
#define OFF 0
#define NULLVAL -9999.0 // MT: define ascii file NULL value as constant
#define MAXPI 20000// limits
#define MAXLEVEL 10
#define MAXHFLOW 10.0
#define MAXSECTIONS 20
#define DEFAULT_THRESHOLD 0.15
#define DEFAULT_CQ 0.432                  //0.432   --20200728
#define DEFAULT_WEIR_COEFF 0.167          //0.167  --20200728
#define DEFAULT_ORIFICE_AREA 0.015        //0.015   --20200728
#define DEFAULT_WEIR_B 0.3				//0.8 --20200728
#define EXPORT_API __declspec(dllexport)
#define INFINIT 100000000
#define MIN_DOUBLE 1e-10
//#define CHKINTERVAL 1.0 // default checkpoint interval in hours runtime
using namespace std;

//__declspec(dllexport) int urbanFloodModel(string path);

#ifdef __cplusplus
extern "C" {
#endif 

	///
	/// @brief ��������Ҫ��¶�Ľӿں���
	///
	EXPORT_API int urbanFloodModel(char*);
	EXPORT_API void test();

#ifdef __cplusplus
}
#endif 



class QModel;
class Controls;
class Domain;
class ParFile;

class ParFile {
public:
	int modelRows=0;
	int modelCols=0;
	double modelBlx=0.0;
	double modelBly=0.0;
	double modelDx=0.0;
	double modelNullValue=-9999.0;
	string saveDir="";
	string resultRoot="res";
	string sectionRoot="";
	int sectionCount=1;
	int simTime=0;
	int saveInterval= INFINIT;
	int initSaveTime=0;
	int initTimeStep=10;
	int rainfall=OFF;
	int radarRain=OFF;
	int rainGage = OFF;
	string rainfallFile;
	int distributedEvap=OFF;
	int evaporation = OFF;
	string evaporationFile;
	int distributedManning=OFF;
	double uniformManning=NULLVAL;
	int infiltration = OFF;
	int distributedInfil = OFF;
	double uniformInfil=0.0;
	int earlyLoss = OFF;
	int distributedLoss = OFF;
	double uniformLoss=0.0;
	int acceleration = OFF;
	int routing = OFF;
	double routingSpeed = 0.0;
	double depthThresh = 0.1;
	double theta=1.0;
	double cfl=0.7;
	double g = 9.8;
	string inpPath;
	int swmm = OFF;
	int swmmSave =ON;
	int maxH=OFF;
	int maxElev=OFF;
	int monitor = OFF;
	int allowPonding = OFF;
	string monitorPath;
	int uniformRules = OFF;
	string rulesPath;
	int customRules = OFF;
	int reportNode = OFF;
	string nodeReportPath;
public:
	/*ParFile();*/
	int init(string);
};



//��ά����
class Matrix2d {
public:
	string fileName;
	string fileType;
	string id;
	double blx;
	double bly;
	double dx;
	int rows;
	int cols;
	double noValue;
	double **arr;
public:
	Matrix2d(string path);
public:
	~Matrix2d();
};//����DEM��n��



//���̵߳�Ԫ
class SpatialCell {
public:
	int nFloor;
	string *floorId;
	double *values;
	SpatialCell *eastCell;
	SpatialCell *southCell;
	SpatialCell *westCell; 
	SpatialCell *northCell;
public:
	SpatialCell();
public:
	void addValue(double,string);
	int clear();
};//�̵߳�Ԫ��ˮ�Ԫ��ˮλ��Ԫ��n��Ԫ


//��Ƶ�����ռ�ṹ
class SpatialStructure {
public:
	double blx;
	double bly;
	double dx;
	int rows;
	int cols;
	double noValue;
	SpatialCell **cellArray;
public:
	SpatialStructure(int, int, double, double, double, double);
	SpatialStructure();
public:
	int appendMatrix2d(Matrix2d &);
	void sort();
	void zerosLike(SpatialStructure&);
	void nullValLike(SpatialStructure&);
	void clone(SpatialStructure&);
	void valuePlus(SpatialStructure&);
	int  finalize();
};

//���㵥Ԫ
class CalcCell {
public:
	int nFloor;
	double *values;
};

//����
class Rainfall {
public:
	double startTime;
	double endTime;
	int index;
	//int count;//������Ը�Ϊstatic�ı��� 2020.7.16 �Ѹ�
	double rain_intensity;
	double **radarRain;//һ֡�״ｵ�꣬����������δ���������Ϣ
public:
	Rainfall();
	int clear();
static int count;
};

//Loss(���𡢺���)
class Loss {
public:
	int rows;
	int cols;
	double loss_rate;// m/s
	CalcCell **cellArr;
public:
	Loss();
	int clear();
};

class Evaporation {
public:
	int count;
	int rows;
	int cols;
	int index;
	int startTime;
	int endTime;
	double evap_rate;// m/s
	CalcCell **cellArr;
public:
	Evaporation();
	int clear();
};

//����ṹ
class Domain {
public:
	int rows;
	int cols;
	double blx;
	double bly;
	double dx;
	double noValue;
	SpatialStructure elevModel;
	SpatialStructure waterDepths;
	SpatialStructure waterElevs;
	SpatialStructure mannings;
	SpatialStructure maxH;
	SpatialStructure maxElev;
	Rainfall* rainList;
	Evaporation* evapList;
	Loss infilration;
	Loss earlyLoss;
	//SpatialCell **floorElevs;
	//SpatialCell **waterDepths;//����wd
	//SpatialCell **waterElevs;//we=fe+wd
	//SpatialCell **mannings;

//public:
	//Domain(Matrix2d *[]);
public:
	Domain();
	void loadPar(const ParFile&);
	/*void loadPar(string par_path);*/
	int  loadDEM(const ParFile&);
	void loadRain(const ParFile&);
	void loadRadarRain(const ParFile&);//��ʵ��
	void loadEvap(const ParFile&);//������distributed
	void doRain(const Controls &,const ParFile &);
	void doEvap(const Controls &, const ParFile &);
	void loadInfil(const ParFile&);//������distributed
	void loadEarlyLoss(const ParFile&);//������distributed
	void doInfil(const Controls&, const ParFile &);
	void doEarlyLoss(const Controls&, const ParFile &);
	void updateH(QModel &,QModel &);
	void dryCheck(QModel &,QModel &);
	int finalize();
};



//����ṹ
/*class CalcDomain {
public:
	int mSize;
	int nSize;
	CalcCell **cellArr;
};*///������������Qpoint




class QCell {
public:
	//z0-fn1��ʼ����Ͳ��ٸı�ָ��
	SpatialCell *z0;
	SpatialCell *z1;
	SpatialCell *h0;
	SpatialCell *h1;
	SpatialCell *fn0;
	SpatialCell *fn1;
	int nfloor0;
	int nfloor1;
	//double **fnArr;
	double **q;
	int  **boundaryType;
	double **qvect;//Ħ�������ȥ�������� ֱ����q�ļ�����һ������
public:
	QCell();
	static void identifyBoundaryType(QCell *,int,int,double);
	void drycheck(int,char,double);
	//~QCell();
	int clear();
};

class Controls {
public:
	//Time
	time_t startTime;
	time_t endTime;
	time_t currentRealTime;
	double currentSimTime;
	int totalSimTime;
	int saveInterval;
	int saveTime;
	int saveIndex;
	double timeStep;
public:
	Controls();
	void init(const ParFile&);
	double updateTimeStep(SpatialStructure&, const ParFile&);
	void updateCurrentTime();
	void updateSaveTime();
	bool timeToSave();
	bool timeToEndSimulation();
};

class QModel {
public:
	int rows;
	int cols;
	double theta;
	double cfl;
	double fn;
	char type;
	QCell **cellArr;
public:
	QModel();
	void init(const Domain&, const ParFile&,const char);
	void build(double);
	double getQinOrQout(int, int, int, char);
	void updateQvect(char,QModel &);
	void updateQ(Controls&,const ParFile &);
	//~QModel();
	int finalize();
};
 
enum PointStatus {
	Init = 0,
	Over_Flow = 3,
	Weir_Flow = 1,
	Orifice_Flow = 2
};

class DischargeRules {
public:
	int rulesCount=0;
	double* upLimit = NULL;
	double* lowLimit = NULL;
	PointStatus* status=NULL;
	double* factors = NULL;
};

class  CouplePoint {
public:
	//string id;//Node name
	int nIndex;//Node Index
	int xIndex;//DEM col
	int yIndex;//DEM row
	int zIndex = 0;//floor
	double floodDepth;
	double overflowVol;
	double weirB;
	double cqA;
	double pondedA;
	PointStatus status;
	int factorIndex;
	int* rules;//����չ
	double* factors;
	double oldFlow;
	double newFlow;

public:
	void updateStatus(double overflow=0.0);
	void updateDischarge();
	void updatePointDepth();
	void setInflow();

public:
	DischargeRules static uniformRules;
	int static loadUniformRules(string path);
	int static count;
};



