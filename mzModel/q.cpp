#include "mzModel.h"
#include "tools.h"
#include "global.h"
using namespace std;

QModel Qx;//global
QModel Qy;//global

QCell::QCell() {
	this->z0 = this->z1 = this->h0 = this->h1 = this->fn0 = this->fn1 = NULL;
	this->nfloor0 = this->nfloor1 = 0;
	this->q = this->qvect=NULL;
	this->boundaryType = NULL;
	//this->fnArr = NULL;
}

void QCell::identifyBoundaryType(QCell *qcell,int n0,int n1,double maxDh) {
	//先判断z0的最高点和z1的最高点哪个更高，方便后续递归判断边界类型
	if (n0 < 0 || n1 < 0) return;
	if(qcell->z0->values[n0] >= qcell->z1->values[n1])//z0的最高点比z1高
	{
		if (qcell->z0->values[n0] - qcell->z1->values[n1] >= maxDh) {
			qcell->boundaryType[n0][n1] = 2;
		}
		else
		{
			qcell->boundaryType[n0][n1] = 1;
		}
		for (int i = n1 - 1; i >= 0; i--) {
			qcell->boundaryType[n0][i] = 0;
		}
		//n0-1 递归
		QCell::identifyBoundaryType(qcell, n0 - 1, n1, maxDh);
	}
	if (qcell->z0->values[n0] < qcell->z1->values[n1])//z0的最高点比z1低
	{
		if (qcell->z1->values[n1] - qcell->z0->values[n0] >= maxDh) {
			qcell->boundaryType[n0][n1] = 2;
		}
		else
		{
			qcell->boundaryType[n0][n1] = 1;
		}
		for (int i = n0 - 1; i >= 0; i--) {
			qcell->boundaryType[i][n1] = 0;
		}
		//n1-1 递归
		QCell::identifyBoundaryType(qcell, n0, n1-1, maxDh);
	}
}

void QCell::drycheck(int floor, char type, double weight) {
	if (!this->q) return;
	int i, j;
	if (type == '0') {
		for (j = 0; j < this->nfloor1; j++) {
			if (this->q[floor][j] > 0) this->q[floor][j] *= weight;
		}
	}
	if (type == '1') {
		for (i = 0; i < this->nfloor0; i++) {
			if (this->q[i][floor] < 0) this->q[i][floor] *= weight;
		}
	}
}

int QCell::clear() 
{
	int i, j;
	if (this->q) {
		for (i = 0; i < this->nfloor0; i++) {
			delete[] this->q[i];
		}
		delete[] this->q;
	}
	if (this->qvect) {
		for (i = 0; i < this->nfloor0; i++) {
			delete[] this->qvect[i];
		}
		delete[] this->qvect;
	}
	if (this->boundaryType) {
		for (i = 0; i < this->nfloor0; i++) {
			delete[] this->boundaryType[i];
		}
		delete[] this->boundaryType;
	}
	return 0;
}

QModel::QModel() {
	this->rows = this->cols = 0;
	this->theta = this->cfl = this->fn = NULLVAL;
	this->cellArr = NULL;
}


//根据高程模型创建相应的Qx、Qy
void QModel::init(const Domain &d, const ParFile &p,const char type) {
	cout << "Q" << type << " initialized" << endl;
	this->theta = p.theta;//1.0 by default
	this->cfl = p.cfl;//0.7 by default
	this->fn = (p.distributedManning==OFF)? p.uniformManning :NULLVAL;//uniform manning if distributedManning off
	QCell *qcell = NULL;
	this->type = type;
	if (type == 'x') {
		this->rows = d.rows;
		this->cols = d.cols + 1;
		this->cellArr = new QCell*[this->rows];
		for (int i = 0; i < this->rows; i++)
			this->cellArr[i] = new QCell[this->cols];
		#pragma omp parallel for private(qcell)
		for (int i = 0; i < this->rows; i++) {
			for (int j = 0; j < this->cols; j++) {
				qcell = &this->cellArr[i][j];
				//not west boundary
				if (j!=0) {
					qcell->z0 = &d.elevModel.cellArray[i][j - 1];
					qcell->h0 = &d.waterDepths.cellArray[i][j - 1];
					qcell->fn0 = (p.distributedManning == OFF) ? NULL : &d.mannings.cellArray[i][j - 1];
					qcell->nfloor0 = d.elevModel.cellArray[i][j - 1].nFloor;
				}
				//west boundary
				else
				{
					qcell->z0 = qcell->h0 = qcell->fn0 = NULL;
					qcell->nfloor0 = 0;
					//this->cellArr[i][j].nfloor0 = d.elevModel.cellArray[i][j - 1].nFloor;
				}
				//not east boumdary
				if (j != (this->cols-1)) {
					qcell->z1 = &d.elevModel.cellArray[i][j];
					qcell->h1 = &d.waterDepths.cellArray[i][j];
					qcell->fn1 = (p.distributedManning == OFF) ? NULL : &d.mannings.cellArray[i][j];
					qcell->nfloor1 = d.elevModel.cellArray[i][j].nFloor;
				}
				//east boundary
				else
				{
					qcell->z1 = qcell->h1 = qcell->fn1 = NULL;
					qcell->nfloor1 = 0;
					//this->cellArr[i][j].nfloor1 = d.elevModel.cellArray[i][j].nFloor;
				}
			}
		}	
	}
	if (type == 'y') {
		this->rows = d.rows+1;
		this->cols = d.cols;
		this->cellArr = new QCell*[this->rows];
		for (int i = 0; i < this->rows; i++)
			this->cellArr[i] = new QCell[this->cols];
		#pragma omp parallel for private(qcell)
		for (int i = 0; i < this->rows; i++) {
			for (int j = 0; j < this->cols; j++) {
				qcell = &this->cellArr[i][j];
				//not north boundary
				if (i!=0) {
					qcell->z0 = &d.elevModel.cellArray[i-1][j];
					qcell->h0 = &d.waterDepths.cellArray[i-1][j];
					qcell->fn0 = (p.distributedManning == OFF) ? NULL : &d.mannings.cellArray[i-1][j];
					qcell->nfloor0 = d.elevModel.cellArray[i-1][j].nFloor;
				}
				//north boundary
				else
				{
					qcell->z0 = qcell->h0 = qcell->fn0 = NULL;
					qcell->nfloor0 = 0;
					//this->cellArr[i][j].nfloor0 = d.elevModel.cellArray[i][j - 1].nFloor;
				}
				//not south boumdary
				if (i!= (this->rows-1)) {
					qcell->z1 = &d.elevModel.cellArray[i][j];
					qcell->h1 = &d.waterDepths.cellArray[i][j];
					qcell->fn1 = (p.distributedManning == OFF) ? NULL : &d.mannings.cellArray[i][j];
					qcell->nfloor1 = d.elevModel.cellArray[i][j].nFloor;
				}
				//south boundary
				else
				{
					qcell->z1 = qcell->h1 = qcell->fn1 = NULL;
					qcell->nfloor1 = 0;
					//this->cellArr[i][j].nfloor1 = d.elevModel.cellArray[i][j].nFloor;
				}
			}
		}	
	}
}

void QModel::build(double threshold) {
	int i, j;
	QCell *qcell;
	#pragma omp parallel for private(j,qcell)
	for (i = 0; i < this->rows; i++) {
		for (j = 0; j < this->cols; j++) {
			qcell = &this->cellArr[i][j];
			if ((qcell->nfloor0 == 0) || (qcell->nfloor1 == 0)) continue;//qcell->q =qcell->qvect=qcell->boundaryType=NULL
			//if (p.distributedManning == ON) {
			//	qcell->fnArr
			//	for(int m=0;m < qcell->nfloor0;m++)
			//		for (int k = 0; k < qcell->nfloor1; k++) {
			//			qcell->fnArr[m][k] = (qcell->fn0->values[m] + qcell->fn1->values[k])*0.5;//average fn
			//		}
			//}
			qcell->q = new double*[qcell->nfloor0];
			qcell->boundaryType = new int*[qcell->nfloor0];
			qcell->qvect=new double*[qcell->nfloor0];
			for (int m = 0; m < qcell->nfloor0; m++) {
				qcell->q[m] = new double[qcell->nfloor1];
				qcell->qvect[m] = new double[qcell->nfloor1];
				for (int k = 0; k < qcell->nfloor1; k++)
				{
					qcell->q[m][k] = 0.0;//initialize Q value=0
					qcell->qvect[m][k] = 0.0;//initialize Qvect value=0 摩擦项
				}
				qcell->boundaryType[m] = new int[qcell->nfloor1];
			}	
			QCell::identifyBoundaryType(qcell, qcell->nfloor0 - 1, qcell->nfloor1 - 1, threshold);//p.modelDx*0.5
		}
	}		
}

//获取Q模型中某行某列某层的净输入或净输出流量，‘0’代表自身作为输出点，‘1’代表自身作为输入点
double QModel::getQinOrQout(int row, int col, int floor, char io) {
	if (row<0 || row>this->rows - 1 || col<0 || col>this->cols - 1)
	{
		cout << "get a Q value doesn't exist\trow:" << row << "\tcol:" << col << endl;
		return 0.0;
	}
	if (!this->cellArr[row][col].q) return 0.0;
	QCell *qcell=&this->cellArr[row][col];
	double q=0.0;
	if (io == '0') {
		if (floor<0 || floor>(qcell->nfloor0 - 1)) {
			cout << "get a Q value doesn't exist\tfloor:\t" << floor << "\tnfloor0-1:\t" << qcell->nfloor0 - 1 << endl;
			cout << "i:\t" << row << " j:\t" <<col<< endl;
			cout << this->type << endl;
			cout << "io:" << io << endl;
			system("pause");
			return 0.0;
		}
		for (int i = 0; i < qcell->nfloor1; i++)
			q += qcell->q[floor][i];
		return q;
	}
	if (io == '1') {
		if (floor<0 || floor>(qcell->nfloor1 - 1)) {
			cout << "get a Q value doesn't exist\tfloor:\t" << floor << "\tnfloor1-1:\t" << qcell->nfloor1 - 1 << endl;
			cout << "i:\t" << row << " j:\t" << col << endl;
			cout << this->type << endl;
			cout << "io:" << io << endl;
			system("pause");
			return 0.0;
		}
		for (int i = 0; i < qcell->nfloor0; i++)
			q += qcell->q[i][floor];
		return q;
	}
	cout << "getQinorQout() wrong" << endl;
	return 0.0;
}


//根据Qx、Qy更新相应Qvect摩擦项
void QModel::updateQvect(char type,QModel & theOtherQ) {
	int i, j;
	QCell *qcell=NULL;
	//cout << "调用updateQvect" << endl;
	if (type == 'x') {
		
		#pragma omp parallel for private(j,qcell)
		for (i = 0; i < this->rows; i++) {
			double *arr0 = NULL, *arr1 = NULL;
			arr0 = new double[MAXLEVEL];
			arr1 = new double[MAXLEVEL];
			/*qcell = this->cellArr[i];*/
			for (j = 0; j < this->cols; j++) {
				qcell = &this->cellArr[i][j];
				if (!qcell->qvect) continue;
				/*double *arr0 = new double[qcell->nfloor0];
				double *arr1 = new double[qcell->nfloor1];*/
				for (int k = 0; k < qcell->nfloor0; k++) {
					//获取左侧栅格k层的y向流入和流出
					if (j == 0) arr0[k] = 0.0;
					else arr0[k] = theOtherQ.getQinOrQout(i, j - 1, k, '1') + theOtherQ.getQinOrQout(i + 1, j - 1, k, '0');
				}
				for (int k = 0; k < qcell->nfloor1; k++) {
					//获取右侧栅格k层的y向流入和流出
					if (j == this->cols - 1) arr1[k] = 0.0;
					else arr1[k] = theOtherQ.getQinOrQout(i, j, k, '1') + theOtherQ.getQinOrQout(i + 1, j, k, '0');
				}
				for (int m = 0; m < qcell->nfloor0; m++)
					for (int k = 0; k < qcell->nfloor1; k++)
						qcell->qvect[m][k] = sqrt(pow(qcell->q[m][k],2)+pow((arr0[m] + arr1[k]) / 4.0, 2));
				/*delete[] arr0; arr0 = NULL;
				delete[] arr1; arr1 = NULL;*/
				/*++qcell;*/
			}
			delete[] arr0; arr0 = NULL;
			delete[] arr1; arr1 = NULL;
		}
			
	}
	if (type == 'y') {
		#pragma omp parallel for private(j,qcell)
		for (i = 0; i < this->rows; i++) {
			double *arr0 = NULL, *arr1 = NULL;
			arr0 = new double[MAXLEVEL];
			arr1 = new double[MAXLEVEL];
			/*qcell = this->cellArr[i];*/
			for (j = 0; j < this->cols; j++) {
				qcell = &this->cellArr[i][j];
				if (!qcell->qvect) continue;
				/*double *arr0 = new double[qcell->nfloor0];
				double *arr1 = new double[qcell->nfloor1];*/
				for (int k = 0; k < qcell->nfloor0; k++) {
					//获取上侧栅格k层的x向流入和流出
					if (i == 0) arr0[k] = 0.0;
					else arr0[k] = theOtherQ.getQinOrQout(i-1, j, k, '1') + theOtherQ.getQinOrQout(i-1, j+1, k, '0');
				}
				for (int k = 0; k < qcell->nfloor1; k++) {
					//获取下侧栅格k层的x向流入和流出
					if (i == this->rows - 1) arr1[k] = 0.0;
					else arr1[k] = theOtherQ.getQinOrQout(i, j, k, '1') + theOtherQ.getQinOrQout(i , j+1, k, '0');
				}
				for (int m = 0; m < qcell->nfloor0; m++)
					for (int k = 0; k < qcell->nfloor1; k++)
						qcell->qvect[m][k] = sqrt(pow(qcell->q[m][k], 2) + pow((arr0[m] + arr1[k]) / 4.0, 2));
				/*delete[] arr0; arr0 = NULL;
				delete[] arr1; arr1 = NULL;*/
				/*qcell++;*/
			}
			delete[] arr0; arr0 = NULL;
			delete[] arr1; arr1 = NULL;
		}
			
	}
}


void QModel::updateQ(Controls &CTRL,const ParFile &par) {
	if (par.acceleration == ON && par.routing==OFF) {
		int i, j,m,k;
		double hflow,slope,fn;
		double g = par.g, tStep = CTRL.timeStep;
		QCell *qcell=NULL;
		double *h0, *h1, *z0, *z1,*f0,*f1;
		h0 = h1 = z0 = z1 = f0 = f1=NULL;
		if (par.distributedManning == OFF)
		{
			fn = par.uniformManning;
			#pragma omp parallel for private(j,qcell,h0,h1,z0,z1,hflow,slope,m,k)
			for (i = 0; i < this->rows; i++) {
				/*qcell = this->cellArr[i];*/
				for (j = 0; j < this->cols; j++) {
					qcell = &this->cellArr[i][j];
					if (!qcell->q) continue;//没有流量交换的栅格边界
					if (qcell->nfloor0 == 0 || qcell->nfloor1 == 0) 
					{
						cout << "qcell->q不为空,但nfloor0或nfloor1为0" << endl;
						continue;
					}
					h0 = qcell->h0->values;
					h1 = qcell->h1->values;
					z0 = qcell->z0->values;
					z1 = qcell->z1->values;
					for (m = 0; m < qcell->nfloor0; m++) {
						for (k = 0; k < qcell->nfloor1; k++) {
							if (qcell->boundaryType[m][k] == 0) continue;//所有边界类型只要非0，就按连续面用acceleration模式算。后续扩展时对此句进行修改。
							hflow = maxDouble(h0[m] + z0[m], h1[k] + z1[k]) - maxDouble(z0[m], z1[k]);
							hflow = maxDouble(hflow, 0.0);
							hflow = minDouble(hflow, MAXHFLOW);
							if (hflow < par.depthThresh) {
								qcell->q[m][k] = 0.0;  
								continue;
							}
							slope = -(z0[m] + h0[m] - z1[k] - h1[k]) / par.modelDx;
							/*if (std::fabs(slope) > 0.15)
							{
								qcell->q[m][k] = 0.0;
								continue;
							}*/
							//突变点由于slope绝对值较大，导致算出的Q会很大，有一点不合理。
							qcell->q[m][k] = (qcell->q[m][k] - (g*tStep*hflow*slope)) / (1 + g * tStep*hflow*fn*fn*fabs(qcell->qvect[m][k]) / (pow(hflow, (10.0 / 3.0))));
							/*if (m == 0 && k == 0 && fabs(qcell->q[m][k])>0) {
								cout << "i,j,q:" << i << "\t" << j << "\t" << qcell->q[m][k] << endl;
								system("pause");
							}*/
						}
					}
					/*if (i == 27 && j == 1 && modelControl.currentSimTime > 600) {
						cout << Qx.cellArr[27][1].h0->values[1] << endl;
						cout << Qx.cellArr[27][1].h1->values[1] << endl;
						cout << "q[0][0]\t" << Qx.cellArr[27][1].q[0][0] << endl;
						cout << "qtype[0][0]\t" << Qx.cellArr[27][1].boundaryType[0][0] << endl;
						cout << "q[0][1]\t" << Qx.cellArr[27][1].q[0][1] << endl;
						cout << "qtype[0][1]\t" << Qx.cellArr[27][1].boundaryType[0][1] << endl;
						cout << "q[1][0]\t" << Qx.cellArr[27][1].q[1][0] << endl;
						cout << "qtype[1][0]\t" << Qx.cellArr[27][1].boundaryType[1][0] << endl;
						cout << "q[1][1]\t" << Qx.cellArr[27][1].q[1][1] << endl;
						cout << "qtype[1][1]\t" << Qx.cellArr[27][1].boundaryType[1][1] << endl;
						system("pause");
					}*/
					
					/*++qcell;*/
				}
			}
		}
		//待扩展
		else {
			cout << "distributedManning on" << endl;
		}
		
	}
	//待扩展
	else {
		cout << "其他模式的updateQ()" << endl;
	}
}

int QModel::finalize() 
{
	if (!this->cellArr) return -1;
	for (int i = 0; i < this->rows; i++) {
		for (int j = 0; j < this->cols; j++) {
			this->cellArr[i][j].clear();
		}
		delete[] this->cellArr[i];
	}
	delete[] this->cellArr;
	return 0;
}