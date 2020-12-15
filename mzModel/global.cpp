#include <iostream>
#include "global.h"
#include "tools.h"
#include "couple.h"
#include "mzModel.h"
#include <iomanip>

/*
   global.h

extern ParFile parFile;
extern Domain domain;
extern QModel Qx;
extern QModel Qy;
extern Controls modelControl;
*/
using namespace std;



int modelInitialize(string parPath)
{
	string inpPath, rptPath, outPath,temp;
	parFile.init(parPath);         //读取参数文件 *.par
	makeDir(parFile.saveDir); //创建数据保存目录
	for (int i = 1; i <= parFile.sectionCount;i++) makeDir(parFile.saveDir+"\\section#"+std::to_string(i));
	modelControl.init(parFile);  //控制器（运行时间、保存时间...）
	domain.loadPar(parFile);    //模拟区域读取参数文件并初始化
	domain.loadDEM(parFile);   //加载dem,初始化高程、水深、水位模型
	if (parFile.maxH)
	{
		domain.maxH.zerosLike(domain.waterDepths);
		cout << "maxH builded" << endl;
	}
	Qx.init(domain, parFile, 'x'); //东西方向流量矩阵
	Qy.init(domain, parFile, 'y'); //南北方向流量矩阵
	Qx.build(parFile.modelDx*0.5);
	Qy.build(parFile.modelDx*0.5);
	domain.loadRain(parFile);//加载降雨数据
	domain.loadEvap(parFile);//加载蒸发数据
	domain.loadInfil(parFile);//加载下渗数据
	domain.loadEarlyLoss(parFile);//加载初损数据(植被、地表截留的雨量)
	if (parFile.monitor)
	{
		ModelMonitor.loadPoints(parFile.monitorPath);
		ModelMonitor.openPointsOut();
	}

	if (parFile.swmm) {
		inpPath = parFile.inpPath;

		rptPath = inpPath;
		rptPath.replace(inpPath.rfind(".inp"), 4, ".rpt");

		outPath = inpPath;
		outPath.replace(inpPath.rfind(".inp"), 4, ".out");
		loadSWMM();
		swmmOpen(inpPath.c_str(), rptPath.c_str(), outPath.c_str());
		//if (!parFile.swmmSave.compare("TRUE")) swmmStart(1);//保存swmm结果
		//else swmmStart(0);
		swmmStart(parFile.swmmSave);
		initPoints();
		if (parFile.uniformRules)
		{
			CouplePoint::loadUniformRules(parFile.rulesPath);
		}
		setAllowPonding(parFile.allowPonding);//向swmm设置是否允许管点积水
		cout << "allowPonding:" << parFile.allowPonding << endl;
		int swmmTotalSimTime = getSWMMTotalTime();
		if (modelControl.totalSimTime != swmmTotalSimTime) modelControl.totalSimTime = swmmTotalSimTime;
		cout << "\nTotal simulation time:" << modelControl.totalSimTime << "s\n" << endl;
	}

	//loadSWMM();
	//swmmOpen(inpPath.c_str(), rptPath.c_str(), outPath.c_str());
	////if (!parFile.swmmSave.compare("TRUE")) swmmStart(1);//保存swmm结果
	////else swmmStart(0);
	//swmmStart(parFile.swmmSave);
	//initPoints();
	//if (parFile.uniformRules) 
	//{
	//	CouplePoint::loadUniformRules(parFile.rulesPath);
	//}
	//setAllowPonding(parFile.allowPonding);//向swmm设置是否允许管点积水
	//cout << "allowPonding:" << parFile.allowPonding << endl;
	//int swmmTotalSimTime = getSWMMTotalTime();
	//if (modelControl.totalSimTime != swmmTotalSimTime) modelControl.totalSimTime = swmmTotalSimTime;
	//cout << "\nTotal simulation time:" << modelControl.totalSimTime << "s\n" << endl;

	return 0;
}

int modelRun() {
	while (!modelControl.timeToEndSimulation())//是否结束
	{	
		if (modelControl.timeToSave()) 
		{
			saveData();
			modelControl.updateSaveTime();
			cout << "next sava time:" << modelControl.saveTime << endl;
		}
		iterate();
		updateData();
		modelControl.updateCurrentTime();
	}
	saveData();
	return 0;
}

int iterate() {
	int i;
	double overflow,swmmTstep;

	if (parFile.swmm && !parFile.allowPonding && modelControl.currentSimTime > 0.0 )
	{
		noPondingOverflow();//
		//else 
		//{
		//	pondingOverflow();
		//}
	}

	modelControl.timeStep = parFile.initTimeStep;
	modelControl.updateTimeStep(domain.waterDepths, parFile);

	if (parFile.swmm) {
		swmmTstep = getSWMMTstep(DW, parFile.initTimeStep);//求swmm和lisflood的耦合计算时间步长(取二者计算步长的较小值) Dynamic Wave
		modelControl.timeStep = minDouble(modelControl.timeStep, swmmTstep);
	}
	//swmmTstep = getSWMMTstep(DW, parFile.initTimeStep);//求swmm和lisflood的耦合计算时间步长(取二者计算步长的较小值) Dynamic Wave
	//modelControl.timeStep = minDouble(modelControl.timeStep, swmmTstep);

	domain.doRain(modelControl, parFile);
	domain.doEarlyLoss(modelControl, parFile);
	domain.doEvap(modelControl, parFile);
	domain.doInfil(modelControl, parFile);

	if (parFile.swmm) {
		#pragma omp parallel for
		for (i = 0; i < CouplePoint::count; i++)
		{
			if (parFile.allowPonding) QPoints[i].updateStatus(); // 这一句是不是有问题了？？？ 没啥问题！但是逻辑有点乱 需要修改整理用于更新溢流状态的代码
			QPoints[i].updateDischarge();
			QPoints[i].updatePointDepth();
			QPoints[i].setInflow();
		}
		swmmStep(modelControl.timeStep);
	}

	//#pragma omp parallel for
	//for (i = 0; i < CouplePoint::count; i++)
	//{
	//	if (parFile.allowPonding) QPoints[i].updateStatus(); // 这一句是不是有问题了？？？ 没啥问题！但是逻辑有点乱 需要修改整理用于更新溢流状态的代码
	//	QPoints[i].updateDischarge();
	//	QPoints[i].updatePointDepth();
	//	QPoints[i].setInflow();
	//}

	//swmmStep(modelControl.timeStep);

	Qy.updateQvect('y', Qx);
	Qx.updateQvect('x', Qy);
	Qy.updateQ(modelControl, parFile);
	Qx.updateQ(modelControl, parFile);
	domain.dryCheck(Qx, Qy);
	domain.updateH(Qx,Qy);

	return 0;
}

int updateData( ) 
{
	int i, j, m;
	SpatialCell* hcell = NULL;
	SpatialCell* maxHcell = NULL;
	if (parFile.maxH) {
#pragma omp parallel for private(j,m,hcell,maxHcell)
		for (i = 0; i < domain.rows; i++) {
			for (j = 0; j < domain.cols; j++) {
				hcell = &domain.waterDepths.cellArray[i][j];
				maxHcell = &domain.maxH.cellArray[i][j];
				if ( hcell->values && maxHcell->values) {
					for (m = 0; m < hcell->nFloor; m++) {
						if (hcell->values[m] > maxHcell->values[m]) maxHcell->values[m] = hcell->values[m];
					}
				}
			}
		}
	}
	return 0;
}


//"E:\\Experiment\\20200612\\ModelData\\par.txt"
int urbanFloodModel(char* path) {
	clock_t t1, t2;
	t1 = clock();
	modelInitialize(path);
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
	//swmmEnd();
	//swmmClose();
	//FreeLibrary(SwmmDll);
	final();
	return 0;
}

int final() 
{
	domain.finalize();
	Qx.finalize();
	Qy.finalize();
	ModelMonitor.finalize();
	return 0;
}


