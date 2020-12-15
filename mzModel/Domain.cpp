#include "mzModel.h"
#include "tools.h"
#include "global.h"
using namespace std;

Domain domain;//global

Domain::Domain() {
	this->rows = this->cols = 0;
	this->blx = this->bly = this->dx = this->noValue = NULLVAL;
	this->rainList = NULL;
	this->evapList = NULL;
	cout << "\nNew domain is initializing..." << endl;
}

void Domain::loadPar(const ParFile &par) {
	cout << "Domain loading ParFile" << endl;
	this->cols=par.modelCols;
	this->rows=par.modelRows;
	this->blx=par.modelBlx;
	this->bly=par.modelBly;
	this->dx=par.modelDx;
	this->noValue=par.modelNullValue;
	//SpatialStructure(int M, int N, double Blx, double Bly, double Dx, double NoValue)
	this->elevModel = SpatialStructure(this->rows, this->cols, this->blx, this->bly, this->dx, this->noValue);
	this->waterDepths = SpatialStructure(this->rows, this->cols, this->blx, this->bly, this->dx, this->noValue);
	this->waterElevs = SpatialStructure(this->rows, this->cols, this->blx, this->bly, this->dx, this->noValue);
	if(par.maxH) this->maxH = SpatialStructure(this->rows, this->cols, this->blx, this->bly, this->dx, this->noValue);
	if(par.maxElev) this->maxElev = SpatialStructure(this->rows, this->cols, this->blx, this->bly, this->dx, this->noValue);
	if (par.distributedManning == ON) this->mannings = SpatialStructure(this->rows, this->cols, this->blx, this->bly, this->dx, this->noValue);
	this->rainList = NULL;
	this->evapList = NULL;
}

int Domain::loadDEM(const ParFile &par) {
	int i, j;
	string path;
	for (i = 1; i <= par.sectionCount; i++) {
		string id = std::to_string(i);
		path = par.sectionRoot +"\\section#"+id+"\\section#" + id + ".dsm";
		Matrix2d dem(path);
		this->elevModel.appendMatrix2d(dem);
	}
	this->elevModel.sort();
	this->waterDepths.zerosLike(this->elevModel);
	this->waterElevs.nullValLike(this->elevModel);
	return 1;
}

void Domain::loadRain(const ParFile &p) 
{
	if(!p.rainfall) //rainfall off
	{
		cout << "rainfall off" << endl;
		return;
	}

	ifstream rainfile(p.rainfallFile);
	if (p.radarRain) //�����״ｵ������
	{
		/* ����չ */
		/*       */
		cout << "radar rain file is openning" << endl;
	}
	else if (p.rainGage)
	{
		cout << "raingage file is openning" << endl;
		if (!rainfile.is_open())
		{ 
			cout << "rainfall List File open fail\t" << p.rainfallFile << endl; 
			return; 
		}

		int n;
		string timeUnit,pUnit,temp;
		int factor = 1;//��λ����ϵ��
		double pfactor=1.0;//����ǿ�Ȼ���ϵ��
		double start=0.0, value=0.0;

		rainfile >> n >> timeUnit >> pUnit;

		//rainfile >>n>>temp;


		/*cout << "�����ļ�ͷ:"<< timeUnit << " " << n << " " << pUnit << endl;*/

		Rainfall::count = n;//�������ݼ�¼����

		if (!timeUnit.compare("second")) factor = 1;
		if (!timeUnit.compare("minute")) factor = 60;
		if (!timeUnit.compare("hour")) factor = 3600;
		if (!timeUnit.compare("day")) factor = 86400;
		if (!pUnit.compare("mm/min")) pfactor = 1000 * 60;
		if (!pUnit.compare("mm/h")) pfactor = 1000 * 3600;
		/*cout << "rainfall intensity unit is " << pUnit << endl;*/

		this->rainList = new Rainfall[n];
		for (int i = 0; i < n; i++) {
			rainfile >> start >> value;
			//ʱ�䵥λת������ֵ��doubleǿ��תint��
			this->rainList[i].startTime = start * factor;
			this->rainList[i].endTime = (start+1) * factor;
			//this->rainList[i].rain_intensity = value/3600.0/1000.0;//����ǿ����mm/h����Ϊm/s
			this->rainList[i].rain_intensity = value / pfactor;//����ǿ����mm/h��mm/min����Ϊm/s
			this->rainList[i].index = i;
			cout << this->rainList[i].startTime << "s\t" << this->rainList[i].endTime << "s\t" << this->rainList[i].rain_intensity * 1000 * 3600 << "mm/h" << endl;
			/*this->rainList[i].count = n;*/
			start += 1.0;
		}
	}
	else  //ʱ���� start end value
	{
		cout << "rainfall time series file is openning" << endl;
		if (!rainfile.is_open()) { cout << "rainfall List File open fail\t" << p.rainfallFile << endl; return; }

		int n;
		string timeUnit,pUnit;
		int factor = 1;//��λ����ϵ��
		double pfactor = 1.0;//����ǿ�Ȼ���ϵ��
		double start, end, value;

		rainfile >> n >> timeUnit >> pUnit;

		Rainfall::count = n;//�������ݼ�¼����

		if (!timeUnit.compare("second")) factor = 1;
		if (!timeUnit.compare("minute")) factor = 60;
		if (!timeUnit.compare("hour")) factor = 3600;
		if (!timeUnit.compare("day")) factor = 86400;
		if (!pUnit.compare("mm/min")) pfactor = 1000.0 * 60;
		if (!pUnit.compare("mm/h")) pfactor = 1000.0 * 3600;

		this->rainList = new Rainfall[n];
		for (int i = 0; i < n; i++) 
		{
			rainfile >> start >> end >> value;
			//ʱ�䵥λת������ֵ
			this->rainList[i].startTime = start * factor;
			this->rainList[i].endTime = end * factor;
			//this->rainList[i].rain_intensity = value/3600.0/1000.0;//����ǿ����mm/h����Ϊm/s
			this->rainList[i].rain_intensity = value / pfactor;//����ǿ����mm/h����Ϊm/s
			this->rainList[i].index = i;
			//this->rainList[i].count = n;
		}
	}
	
}

//�����״ｵ������
void Domain::loadRadarRain(const ParFile &p) {
	
}

//����������
void Domain::loadInfil(const ParFile &p) {

	if (p.distributedInfil==ON) //distributedInfil
	{
		/* ����չ */
		/*       */
	}
	else //uniformInfil
	{
		this->infilration.cols = this->cols;
		this->infilration.rows = this->rows;
		this->infilration.loss_rate = p.uniformInfil;
	}
}

//���س�����Ҫ��ֲ������������
void Domain::loadEarlyLoss(const ParFile &p) {
	if (p.distributedLoss == ON) //distributedLoss
	{
		/* ����չ */
		/*       */
	}
	else  //uniformLoss
	{
		this->earlyLoss.cols = this->cols;
		this->earlyLoss.rows = this->rows;
		this->earlyLoss.loss_rate = p.uniformLoss;
	}
}

void Domain::loadEvap(const ParFile &p) {
	if (!p.evaporation) {
		cout << "evaporation off" << endl;
		return;
	}
	if (p.distributedEvap == ON) //distributedEvap
	{ 
		/* ����չ */
		/*       */
	}
	else 
	{
		ifstream eList(p.evaporationFile);
		if (!eList.is_open()) {cout << "evaporationFile open fail\t" << p.evaporationFile << endl; return;}
		int n;
		string unit,timeUnit;
		double factor1 = 1.0;//�����ʵ�λ����ϵ��
		int factor2 = 1;//��ֹʱ�䵥λ����ϵ��
		double start, end, value;
		eList>> n >> timeUnit >> unit ;//����������ʱ�䵥λ�������ʵ�λ
		if (!unit.compare("m/s")) factor1 = 1.0;
		if (!unit.compare("mm/h")) factor1 = 3600.0*1000.0;
		if (!timeUnit.compare("second")) factor2 = 1;
		if (!timeUnit.compare("minute")) factor2 = 60;
		if (!timeUnit.compare("hour")) factor2 = 3600;
		if (!timeUnit.compare("day")) factor2 = 86400;
		this->evapList = new Evaporation[n];
		for (int i = 0; i < n; i++) {
			eList >> start >> end >> value;
			//ʱ�䵥λת������ֵ��doubleǿ��תint��
			this->evapList[i].count = n;
			this->evapList[i].index = i;
			this->evapList[i].startTime = start * factor2;
			this->evapList[i].endTime = end * factor2;
			this->evapList[i].evap_rate = value/factor1;//�����ʵ�λ����ΪΪm/s
		}
	}
}

//����
void Domain::doRain(const Controls &CTRL,const ParFile &par) {
	if (!this->rainList) { 
		/*cout << "û�н���ģ�������" << endl; */
		return; 
	}
	int i,m,k,len=Rainfall::count;
	double intensity=0.0;
	double val;
	double dh;
	double currentT = CTRL.currentSimTime;
	double tStep = CTRL.timeStep;
	//
	Rainfall *r = this->rainList;
	int index = -1;
	for (i = 0; i < len; i++) {
		if ( (currentT>= r->startTime) && (currentT < r->endTime) ) //find out rain intensity index according current time
		{
			index = i;
			break;
		}
		else {
			++r;
		}
	}
	if (index!=-1) {
		if (!(par.radarRain)) //uniform rain
		{
			intensity = r->rain_intensity;
			dh = intensity * tStep;
			SpatialStructure *wd = &this->waterDepths;
			SpatialCell *cell = NULL;
			#pragma omp parallel for private(k,cell)
			for(m=0;m<this->rows;m++)
				for (k = 0; k < this->cols; k++) {
					cell = &wd->cellArray[m][k];
					if (cell->nFloor > 0) cell->values[cell->nFloor - 1] += dh;//��߲���ս�����
					/*cout << "������:" << intensity * tStep << endl;*/
				}
		}
		else  //radar rain
		{
			cout << "radar rain" << endl;
		}
	}
}

void Domain::doEvap(const Controls & CTRL, const ParFile &par) {
	if (!this->evapList) {
		/*cout << "û������ģ�������" << endl;*/
		return;
	}
	int i, m, k, len = this->evapList[0].count;
	int index = -1;
	double evapRate = 0.0;
	Evaporation *e = this->evapList;
	double currentT = CTRL.currentSimTime;
	double tStep = CTRL.timeStep;
	for (i = 0; i < len; i++) {
		if ( (currentT >= e->startTime) && ( currentT < e->endTime ) ) //find out Evap. intensity index according current time
		{
			index = i;
			break;
		}
		else {
			++e;
		}
	}
	if (index != -1) {
		if (!(par.distributedEvap)) //uniform evap
		{
			evapRate = e->evap_rate;
			SpatialStructure *wd = &this->waterDepths;
			SpatialCell *cell = NULL;
			#pragma omp parallel for private(k,cell)
			for (m = 0; m < this->rows; m++)
				for (k = 0; k < this->cols; k++) {
					cell = &wd->cellArray[m][k];
					if (cell->nFloor > 0) {
						for (int v = 0; v < cell->nFloor; v++) {
							cell->values[v] -= evapRate * tStep;
							if (cell->values[v] < 0.0) cell->values[v] = 0.0;
						}
	
					}
					
				}
		}
		else  //distributed Evap
		{
			cout << "distributed Evap" << endl;
		}
	}
}

void Domain::doInfil(const Controls & CTRL, const ParFile &par) {
	if (par.distributedInfil) {
		cout << "distributed Infil is on" << endl;
	}
	else if (this->infilration.loss_rate<=0.0) {
		return;
	}
	else  //uniform
	{	
		int i, j;
		double infilRate = this->infilration.loss_rate;
		double tStep = CTRL.timeStep;
		SpatialStructure *wd = &this->waterDepths;
		SpatialCell *cell = NULL;
		#pragma omp parallel for private(j,cell)
		for(i=0;i<this->rows;i++)
			for (j = 0; j < this->cols; j++) {
				cell = &(wd->cellArray[i][j]);
				if (cell->nFloor > 0) {
					for (int m = 0; m < cell->nFloor; m++) {
						cell->values[m] -= infilRate * tStep;
						if (cell->values[m] < 0) cell->values[m] = 0.0;
					}
				}
			}
	}
}

void Domain::doEarlyLoss(const Controls & CTRL, const ParFile &par) {
	if (par.distributedInfil) {
		cout << "distributed EarlyLoss is on" << endl;
	}
	else if (this->earlyLoss.loss_rate<=0.0) {
		return;
	}
	else //uniform
	{
		int i, j;
		double lossRate = this->earlyLoss.loss_rate;
		double tStep = CTRL.timeStep;
		SpatialStructure *wd = &this->waterDepths;
		SpatialCell *cell = NULL;
		#pragma omp parallel for private(j,cell)
		for (i = 0; i < this->rows; i++)
			for (j = 0; j < this->cols; j++) {
				cell = &(wd->cellArray[i][j]);
				if (cell->nFloor > 0) {
					cell->values[cell->nFloor - 1] -= lossRate * tStep;//��߲��������
					if (cell->values[cell->nFloor - 1] < 0.0) cell->values[cell->nFloor - 1] = 0.0;
				}
			}
	}
}
void Domain::updateH(QModel &Qx, QModel &Qy) {
	int i, j,m;
	double a = this->dx*this->dx;
	double q = 0.0;
	SpatialCell *hCell = NULL;
#pragma omp parallel for private(j,hCell,q)
	for (i = 0; i < this->rows; i++) {
		for (j = 0; j < this->cols; j++) {
			hCell = & (this->waterDepths.cellArray[i][j]);
			if (hCell->nFloor > 0) {
				for (int m = 0; m < hCell->nFloor; m++) {
					//�ۼӱ����ϣ��������ĸ���������
					q = Qy.getQinOrQout(i, j, m, '1') - Qy.getQinOrQout(i + 1, j, m, '0') + Qx.getQinOrQout(i, j, m, '1') - Qx.getQinOrQout(i, j + 1, m, '0');
					hCell->values[m] += q / a * modelControl.timeStep * this->dx;
					if (hCell->values[m] < 0.0) hCell->values[m] = 0.0;
				}
			}
		}
	}		
}

void Domain::dryCheck(QModel &Qx, QModel &Qy) {
	int i, j, m;
	double vol,q, dV, WDweight;
	double A = this->dx*this->dx;
	SpatialCell *cell = NULL;
	SpatialStructure &wd = this->waterDepths;
	for (i = 0; i < this->rows; i++) 
	{
		cell = wd.cellArray[i];
		for (j = 0; j < this->cols; j++) 
		{
			if (cell->nFloor > 0) {
				for (m = (cell->nFloor-1); m >=0 ;m--) {
					vol = cell->values[m] * A;
					q= (Qy.getQinOrQout(i, j, m, '1') - Qy.getQinOrQout(i + 1, j, m, '0') + Qx.getQinOrQout(i, j, m, '1') - Qx.getQinOrQout(i, j + 1, m, '0')) * this->dx;
					dV = q * modelControl.timeStep;
					if (vol + dV < 0) {
						WDweight = vol / (-dV);
						Qy.cellArr[i][j].drycheck(m, '1', WDweight);
						Qy.cellArr[i+1][j].drycheck(m, '0', WDweight);
						Qx.cellArr[i][j].drycheck(m, '1', WDweight);
						Qx.cellArr[i][j+1].drycheck(m, '0', WDweight);
					}
				}
			}
			++cell;
		}
	}
		
}

int Domain::finalize() 
{
	//�ͷ���Ӧģ��Ķ����ڴ�
	int n;
	this->elevModel.finalize();
	this->waterDepths.finalize();
	this->waterElevs.finalize();
	this->mannings.finalize();
	this->maxH.finalize();
	if (this->rainList) {
		n = this->rainList[0].count;
		for (int i = 0; i < n; i++)
			this->rainList[i].clear();
		delete[] this->rainList; this->rainList = NULL;
	}
	if (this->evapList) {
		n = this->evapList[0].count;
		for (int i = 0; i < n; i++) {
			this->evapList[i].clear();
		}
		delete[] this->evapList; this->evapList = NULL;
	}
	this->infilration.clear();
	this->earlyLoss.clear();
	return 0;
}
