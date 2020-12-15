/*
	load Swmm Dll
	load coupling functions
*/

#include "couple.h"

using namespace std;

//SWMM functions
HINSTANCE SwmmDll=NULL;
GetCouplePointsN getCouplePointsN=NULL;
GetCouplePoints getCouplePoints=NULL;
SWMM_OPEN swmmOpen=NULL;
SWMM_START swmmStart=NULL;
GetSWMMTotalTime getSWMMTotalTime = NULL;
SWMM_STEP swmmStep = NULL;
SWMM_END swmmEnd = NULL;
SWMM_CLOSE swmmClose = NULL;
GetOverflow getOverflow = NULL;
SetLatFlow setLatFlow = NULL;
GetSWMMTstep getSWMMTstep = NULL;
SetAllowPonding setAllowPonding = NULL;
GetNodeHead getNodeHead = NULL;
ReportNodeFlood reportNodeFlood=NULL;

//declare relevant variables
DischargeRules  CouplePoint::uniformRules;
CouplePoint* QPoints=NULL;
int CouplePoint::count=0;

//int NCouplePoint;

int loadSWMM() {
	SwmmDll = LoadLibrary(TEXT("SwmmSourceCode.dll"));
	swmmOpen = (SWMM_OPEN)GetProcAddress(SwmmDll, "swmm_open");
	swmmStart = (SWMM_START)GetProcAddress(SwmmDll, "swmm_start");
	swmmStep = (SWMM_STEP)GetProcAddress(SwmmDll, "swmm_step");
	swmmEnd = (SWMM_END)GetProcAddress(SwmmDll, "swmm_end");
	swmmClose = (SWMM_CLOSE)GetProcAddress(SwmmDll, "swmm_close");
	getCouplePointsN = (GetCouplePointsN)GetProcAddress(SwmmDll, "swmm_getCouplePointsN");
	getCouplePoints = (GetCouplePoints)GetProcAddress(SwmmDll, "swmm_getCouplePoints");
	getSWMMTotalTime = (GetSWMMTotalTime)GetProcAddress(SwmmDll, "swmm_getSWMMSimTime");
	getOverflow = (GetOverflow)GetProcAddress(SwmmDll, "swmm_getOverflow");
	setLatFlow = (SetLatFlow)GetProcAddress(SwmmDll, "swmm_setLatFlow");
	getSWMMTstep = (GetSWMMTstep)GetProcAddress(SwmmDll, "routing_getRoutingStep");
	setAllowPonding = (SetAllowPonding)GetProcAddress(SwmmDll, "swmm_setOption_allowPonding");
	getNodeHead = (GetNodeHead)GetProcAddress(SwmmDll, "swmm_getNodeHead");
	reportNodeFlood=(ReportNodeFlood)GetProcAddress(SwmmDll, "swmm_nodeFlood");

	if (!SwmmDll) {
		cout << "load swmm fail" << endl; 
		return -1;
	}
	if (!swmmOpen) {
		cout << "load swmm_open fail" << endl;
		return -1;
	}
	if (!swmmStart) {
		cout << "load swmm_start fail\n" << endl;
		return -1;
	}
	if (!swmmStep) {
		cout << "load swmm_step fail\n" << endl;
		return -1;
	}
	if (!swmmClose) {
		cout << "load swmm_close fail\n" << endl;
		return -1;
	}
	if (!swmmEnd) {
		cout << "load swmm_end fail\n" << endl;
		return -1;
	}
	if (!getCouplePointsN) {
		cout << "load swmm_getCouplePointsN fail" << endl;
		return -1;
	}
	if (!getCouplePoints) {
		cout << "load swmm_getCouplePoints fail" << endl;
		return -1;
	}
	if (!getSWMMTotalTime) {
		cout << "load swmm_getSWMMSimTime fail" << endl;
		return -1;
	}
	if (!getOverflow) {
		cout << "load swmm_getOverflow fail" << endl;
		return -1;
	}
	if (!setLatFlow) {
		cout << "load swmm_setLatFlow fail" << endl;
		return -1;
	}
	if (!getSWMMTstep) {
		cout << "load routing_getRoutingStep fail" << endl;
		return -1;
	}
	if (!setAllowPonding) {
		cout << "load swmm_setOption_allowPonding fail" << endl;
		return -1;
	}
	if (!getNodeHead) {
		cout << "load swmm_getNodeHead fail" << endl;
		return -1;
	}
	if (!reportNodeFlood) {
		cout << "load reportNodeFlood fail" << endl;
		return -1;
	}

	cout << "loadSWMM success" << endl;//成功加载所有swmm接口函数
	return 0;
}

int initPoints() {
	int n = 0;
	/*NCouplePoint = 0;*/
	n= getCouplePointsN(domain.blx, domain.bly, domain.rows, domain.cols, domain.dx);
	int* indexs = new int[n];
	int* rows = new int[n];
	int* cols = new int[n];
	double* cqAs = new double[n];
	double* weirBs = new double[n];
	double* pondedAs = new double[n];
	/*NCouplePoint=getCouplePoints(domain.blx, domain.bly, domain.rows, domain.cols, domain.dx, indexs, rows, cols, cqAs, weirBs);*/
	CouplePoint::count = getCouplePoints(domain.blx, domain.bly, domain.rows, domain.cols, domain.dx, indexs, rows, cols, cqAs, weirBs, pondedAs);
	if (CouplePoint::count != n) {
		cout << "couple points count wrong" << endl;
		return -1;
	}
	QPoints = (CouplePoint*)calloc(n, sizeof(CouplePoint));
	if (!QPoints) {
		cout << "calloc QPoints fail" << endl;
		return -1;
	}
	for (int i = 0; i < n; i++) {
		QPoints[i].nIndex = indexs[i];
		QPoints[i].yIndex = rows[i];
		QPoints[i].xIndex = cols[i];
		QPoints[i].zIndex = 0;
		QPoints[i].cqA = cqAs[i] > 0.0 ? cqAs[i] : DEFAULT_ORIFICE_AREA;
		QPoints[i].weirB = weirBs[i] > 0.0 ? weirBs[i] : DEFAULT_WEIR_B;
		QPoints[i].floodDepth = 0.0;
		QPoints[i].oldFlow = 0.0;
		QPoints[i].newFlow = 0.0;
		QPoints[i].overflowVol = 0.0;
		QPoints[i].status = Init;
		QPoints[i].rules = NULL;
		QPoints[i].factorIndex = -1;
		QPoints[i].factors = NULL;
		QPoints[i].pondedA = pondedAs[i];
		/*cout << "耦合点" << QPoints[i].nIndex << "\tCQA:" << QPoints[i].cqA << "\tpondedA:" << QPoints[i].pondedA << endl;*/
	}
	delete[] indexs; indexs = NULL;
	delete[] rows; rows = NULL;
	delete[] cols; cols = NULL;
	delete[] cqAs; cqAs = NULL;
	delete[] weirBs; weirBs = NULL;
	cout << "\ninit Points success" << endl;
	return 0;
}


void noPondingOverflow() 
{
	int i;
	double overflow;
	for (i = 0; i < CouplePoint::count; i++) {
		overflow = getOverflow(QPoints[i].nIndex);
		QPoints[i].updateStatus(overflow);
	}
}

void pondingOverflow() //！暂未使用
{
	int i;
	for (i = 0; i < CouplePoint::count; i++) 
	{
		QPoints[i].updateStatus();//不传入参数
	}
	
}


void CouplePoint::updateStatus(double overflow)
{
	if (!domain.waterDepths.cellArray[this->yIndex][this->xIndex].values)
		return;//耦合点无高程值

	if (!parFile.allowPonding) //服务于noponding overflow，不允许雨水井积水时，使用swmm计算的overflow流量，更新溢流总体积。
	{
		double floodDepth;
		
		if (overflow > 0.0)
		{
			this->overflowVol = overflow * modelControl.timeStep; //计算溢流量
		}
		else
		{
			this->overflowVol = 0.0;
		}

		this->floodDepth = domain.waterDepths.cellArray[this->yIndex][this->xIndex].values[this->zIndex];
	
	}
	else   //服务于ponding overflow，不允许雨水井积水时，更新水头差
	{
		this->overflowVol = 0.0;
		int overFlowFlag;
		double head = getNodeHead(this->nIndex, &overFlowFlag);
		if (overFlowFlag) 
		{
			this->floodDepth = domain.waterDepths.cellArray[this->yIndex][this->xIndex].values[this->zIndex] +
				domain.elevModel.cellArray[this->yIndex][this->xIndex].values[this->zIndex] - head;                    // 地表水位=水深+地表高程， 水头差=地表水位-管点积水水位
		}
		else 
		{
			this->floodDepth = domain.waterDepths.cellArray[this->yIndex][this->xIndex].values[this->zIndex];         //管点不溢流，则水头差为地表积水水深，而非减去管点水位。
		}
	}
}

int getRulesIndex(double depth) 
{
	int i,index=-1;
	for (i = 0; i < CouplePoint::uniformRules.rulesCount; i++) {
		if (depth >= CouplePoint::uniformRules.lowLimit[i] && depth < CouplePoint::uniformRules.upLimit[i]) {
			index = i; break;
		}
	}
	return index;
}

void CouplePoint::updateDischarge() 
{
	double floodDepth,maxVol;
	int ruleIndex;
	int default_flag;
	if (!domain.waterDepths.cellArray[this->yIndex][this->xIndex].values) //耦合点无高程值
	{
		return;
	}
	this->oldFlow = this->newFlow;

	if (!parFile.allowPonding && (this->overflowVol > MIN_DOUBLE)) //overflow
	{
		this->newFlow = 0.0;
	}
	else if (this->floodDepth < 0.0)// 注意，allowPonding==OFF时，floodDepth一定大于等于0。所以allowPonding==OFF&& overflowVol==0时，只会走discharge
	{
		/*default_flag = 1;*/
		/*this->newFlow = 0.0;*/
		floodDepth = -1.0*this->floodDepth;
		this->overflowVol = floodDepth * this->pondedA;
		this->newFlow = this->overflowVol / modelControl.timeStep;
	}

	else //discharge
	{
		default_flag = 1;

		/*floodDepth = domain.waterDepths.cellArray[this->yIndex][this->xIndex].values[this->zIndex];*/
		floodDepth = this->floodDepth;
		maxVol = floodDepth * domain.dx*domain.dx;

		if (parFile.uniformRules) //uniform rules 所有节点使用统一的流态排水规则
		{

			ruleIndex = getRulesIndex(floodDepth);

			if (ruleIndex == -1);//not found rule,use default

			else
			{
				switch (CouplePoint::uniformRules.status[ruleIndex])
				{
				case Weir_Flow:
					this->newFlow = (-1.0)*CouplePoint::uniformRules.factors[ruleIndex] * this->weirB*pow(floodDepth, 1.5)*pow(2 * parFile.g, 0.5);
					break;
				case Orifice_Flow:
					this->newFlow = (-1.0)*CouplePoint::uniformRules.factors[ruleIndex] * this->cqA*pow(2 * parFile.g*floodDepth, 0.5);
					break;
				default:
					this->newFlow = 0.0;
					break;

				}

				default_flag = 0;//not use default
			}

		}
		else if (parFile.customRules)//custom rules
		{
			/*待扩展*/
			/*待扩展*/
			default_flag = 0;//not use default
		}

		if ( default_flag ) //default rules
		{
			/*floodDepth = domain.waterDepths.cellArray[this->yIndex][this->xIndex].values[this->zIndex];*/
			floodDepth = this->floodDepth;
			maxVol = floodDepth * domain.dx*domain.dx;
			if (floodDepth > 0 && floodDepth <= DEFAULT_THRESHOLD)
			{
				this->newFlow = (-1.0)*DEFAULT_WEIR_COEFF * this->weirB*pow(floodDepth, 1.5)*pow(2 * parFile.g, 0.5);
			}
			if (floodDepth > DEFAULT_THRESHOLD)
			{
				this->newFlow = (-1.0)*DEFAULT_CQ * this->cqA*pow(2 * parFile.g*floodDepth, 0.5);
			}
		}

		//this->newFlow = (this->newFlow + this->oldFlow) / 2.0;//线性插值取平均

		if (maxVol < this->newFlow*modelControl.timeStep) //防止排水量大于栅格内水量
		{
			this->newFlow = (-1.0)*maxVol / modelControl.timeStep;
		}
			
	}


}

void CouplePoint::updatePointDepth() //排水,更新地面水深
{
	double* wd = domain.waterDepths.cellArray[this->yIndex][this->xIndex].values;
	double a;
	if (wd) {
		a = pow(domain.dx, 2);      //栅格面积
		if (this->overflowVol > 0.0) //overflow
		{
			wd[this->zIndex] += this->overflowVol / a;
			//this->overflowVol = 0.0;                    //重置为0.0
		}
		else 
		{
			wd[this->zIndex] += this->newFlow*modelControl.timeStep / a;
			/*wd[this->zIndex] += ( (this->newFlow + this->oldFlow)/2.0*modelControl.timeStep/a );*/
		}
		
	}
}

void CouplePoint::setInflow() 
{
	setLatFlow(this->nIndex, -1.0*this->newFlow);
}

int CouplePoint::loadUniformRules(string path) {
	ifstream rulesFile(path);
	string temp;
	int count=0;
	if (!rulesFile) {
		cout << "uniformRules File open fail" << endl;
		return -1;
	}
	rulesFile >> temp >> count;
	CouplePoint::uniformRules.rulesCount = count;
	/*do {
		getline(rulesFile, temp);
	} while (*temp.c_str() != ';');*/
	
	double* upls=
		CouplePoint::uniformRules.upLimit = new double[count];

	double* lowls=
		CouplePoint::uniformRules.lowLimit = new double[count];

	PointStatus* pss=
		CouplePoint::uniformRules.status = new PointStatus[count];

	double* fcts=
		CouplePoint::uniformRules.factors = new double[count];

	//读取rules
	cout << "\nUniform Rules:" << endl;
	for (int i = 0; i < count; i++) 
	{

		rulesFile >> lowls[i] >> upls[i] >> temp >> fcts[i];//读取一行记录

		if (!temp.compare("WEIR")) pss[i] = Weir_Flow;
		else if (!temp.compare("ORIFICE")) pss[i] = Orifice_Flow;
		else pss[i] = Init;
		cout << "\tlow:" << lowls[i] << "\tup:" << upls[i] << "\tstatus:" << pss[i] << "\tfactor:" << fcts[i] << endl;
	}
	rulesFile.close();
	return 0;
}

