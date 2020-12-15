#include "mzModel.h"
#include "tools.h"
#include "global.h"
using namespace std;

Controls modelControl;//global

Controls::Controls() {
	this->currentRealTime = this->timeStep = NULLVAL;
	this->totalSimTime = this->saveInterval = this->saveTime =this->saveIndex= 0;
}
void Controls::init(const ParFile &par) {
	this->totalSimTime = par.simTime;
	this->timeStep = par.initTimeStep;
	this->currentSimTime = 0.0;
	this->saveInterval = par.saveInterval;
	this->saveTime = par.initSaveTime;
	this->saveIndex = 0;
}

//更新计算步长
double Controls::updateTimeStep(SpatialStructure &wds, const ParFile& par) {
	/*this->timeStep = 10.0;
	return 10.0;*/
	double maxH = 0.0;
	double dt = 0.0;
	//acceleration模式下的步长计算方法
	if (par.acceleration == ON) {
		int i, j;
		double  threadMaxH;
		double locT;
		SpatialCell* p = NULL;
		#pragma omp parallel for private(j,p,threadMaxH)
		for (i = 0; i < wds.rows; i++) {
			threadMaxH = 0.0;
			for (j = 0; j < wds.cols; j++) {
				p = &wds.cellArray[i][j];
				if (p->nFloor >0 && threadMaxH<arrMaxdouble(p->values, p->nFloor)){
					threadMaxH = arrMaxdouble(p->values, p->nFloor);
				}
			}
			#pragma omp critical
			{
				if (threadMaxH > maxH)
				{
					 maxH = threadMaxH;
				}
			}
		}
		if (maxH > par.depthThresh) {
			locT = par.cfl * wds.dx / (sqrt(par.g*maxH));
			/*cout << "locT:"<<locT << endl;*/
			if (this->timeStep > locT)this->timeStep = locT;
		}
		else
		{
			this->timeStep = par.initTimeStep;
		}
	}
	//后续进行扩展其他步长计算方法
	else
	{
		this->timeStep = par.initTimeStep;
	}

	//dt = this->totalSimTime - this->currentSimTime;
	//if (this->timeStep > dt) this->timeStep = dt;
	this->timeStep = minDouble(this->totalSimTime - this->currentSimTime, this->timeStep);
	if (this->currentSimTime > 0.0) this->timeStep = minDouble(this->saveTime - this->currentSimTime, this->timeStep);

	return this->timeStep;
		
}

//是否是保存一次数据的时间
bool Controls::timeToSave() {
	if (this->currentSimTime >= this->saveTime) { 
		/*cout << "保存时间到" << this->saveTime << endl; */
		return true;
	}
	else { 
		return false;
	}
}

//更新下一次保存结果的时间
void Controls::updateSaveTime() {
	this->saveTime += this->saveInterval;
	this->saveIndex += 1;
}

//更新当前模拟时间和真实时间戳
void Controls::updateCurrentTime() {
	this->currentSimTime += this->timeStep;
	this->currentRealTime = time(NULL);
}

//模拟结束
bool Controls::timeToEndSimulation() {
	if (this->currentSimTime >= this->totalSimTime) {
		return true;
	}
	else
	{
		return false;
	}
}