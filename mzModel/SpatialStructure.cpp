#include "mzModel.h"
#include <iomanip>

SpatialStructure::SpatialStructure() {
	this->cellArray = NULL;
}

SpatialStructure::SpatialStructure(int M, int N, double Blx, double Bly, double Dx, double NoValue) {
	this->rows = M;
	this->cols = N;
	this->blx = Blx;
	this->bly = Bly;
	this->dx = Dx;
	this->noValue = NoValue;
	this->cellArray = new SpatialCell*[this->rows];
	for (int i = 0; i < this->rows; i++)
		this->cellArray[i] = new SpatialCell[this->cols];
#pragma omp parallel for
	for (int i = 0; i < this->rows; i++) {
		for (int j = 0; j < this->cols; j++)
		{
			this->cellArray[i][j].eastCell = (j == this->cols - 1) ? NULL : &this->cellArray[i][j + 1];
			this->cellArray[i][j].westCell = (j == 0) ? NULL : &this->cellArray[i][j - 1];
			this->cellArray[i][j].northCell = (i == 0) ? NULL : &this->cellArray[i-1][j];
			this->cellArray[i][j].southCell = (i == this->rows-1) ? NULL : &this->cellArray[i+1][j];
		}
	}
	cout << "new spatial structure builded" << endl;
}

int SpatialStructure::appendMatrix2d(Matrix2d & matrix) {
	int flag,i,j;
	cout << "Spatial Structure data:  ";
	cout << this->rows << "  " << this->cols << "  ";
	cout << setprecision(12) << this->blx << "  " << this->bly << "  " << this->dx << "  " << this->noValue << endl;
	flag = (this->blx == matrix.blx) && (this->bly == matrix.bly) && (this->dx == matrix.dx)&&
		 (this->rows == matrix.rows) && (this->cols == matrix.cols);
	if (!flag) { cout << matrix.fileName << "  Exception:matrix shape wrong" << endl; return 0; }
	for (i=0;i<this->rows;i++)
		for (j = 0; j < this->cols; j++) {
			if (matrix.arr[i][j] != NULLVAL) {
				this->cellArray[i][j].addValue(matrix.arr[i][j],matrix.id);
			}
		}
	return 1;
}

void SpatialStructure::sort() {
	int i, j, k,m,len;
	bool didSwap;
	double temp;
	string strtemp;
	double* arr;
	string* ids;
#pragma omp parallel for private(j,k,m,len,didSwap,temp,strtemp,arr,ids)
	for (i = 0; i < this->rows; i++) {
		for (j = 0; j < this->cols; j++) {
			len = this->cellArray[i][j].nFloor;
			arr = this->cellArray[i][j].values;
			ids = this->cellArray[i][j].floorId;
			for (k = 0; k < len; k++) {
				didSwap = true;
				for (m = 0; m < len - k - 1; m++) {
					if (arr[m + 1] < arr[m]) {
						temp = arr[m];
						strtemp = ids[m];
						arr[m] = arr[m + 1];
						ids[m] = ids[m + 1];
						arr[m + 1] = temp;
						ids[m + 1] = strtemp;
						didSwap = false;
					}
				}
				if (didSwap) break;
			}
		}
	}
}

void SpatialStructure::zerosLike(SpatialStructure &ss) {
	this->rows = ss.rows;
	this->cols = ss.cols;
	this->blx = ss.blx;
	this->bly = ss.bly;
	this->dx = ss.dx;
	this->noValue = ss.noValue;
	int i, j, m, length;
	SpatialCell *p,*target;
#pragma omp parallel for private(j,m,length,p,target)
	for (i = 0; i < this->rows; i++) {
		for (j = 0; j<this->cols; j++) {
			p = &this->cellArray[i][j];
			target = &ss.cellArray[i][j];
			length = target->nFloor;
			p->nFloor = length;
			if (length > 0) {
				p->values = new double[length];
				p->floorId = new string[length];
				for (m = 0; m < length; m++) {
					p->values[m] = 0.0;
					p->floorId[m] = target->floorId[m];
				}
			}
		}
	}
}

void SpatialStructure::nullValLike(SpatialStructure &ss) {
	this->rows = ss.rows;
	this->cols = ss.cols;
	this->blx = ss.blx;
	this->bly = ss.bly;
	this->dx = ss.dx;
	this->noValue = ss.noValue;
	int i, j, m, length;
	SpatialCell *p, *target;
#pragma omp parallel for private(j,m,length,p,target)
	for (i = 0; i < this->rows; i++) {
		for (j = 0; j < this->cols; j++) {
			p = &this->cellArray[i][j];
			target = &ss.cellArray[i][j];
			length = target->nFloor;
			p->nFloor = length;
			if (length > 0) {
				p->values = new double[length];
				p->floorId = new string[length];
				for (m = 0; m < length; m++) {
					p->values[m] = NULLVAL;
					p->floorId[m] = target->floorId[m];
				}
			}
		}
	}
}

void SpatialStructure::clone(SpatialStructure &ss) {
	this->rows = ss.rows;
	this->cols = ss.cols;
	this->blx = ss.blx;
	this->bly = ss.bly;
	this->dx = ss.dx;
	this->noValue = ss.noValue;
	int i, j, m, length;
	SpatialCell *p, *target;
#pragma omp parallel for private(j,m,length,p,target)
	for (i = 0; i < this->rows; i++) {
		for (j = 0; j < this->cols; j++) {
			p = &this->cellArray[i][j];
			target = &ss.cellArray[i][j];
			length = target->nFloor;
			p->nFloor = length;
			if (length > 0) {
				p->values = new double[length];
				p->floorId = new string[length];
				for (m = 0; m < length; m++) {
					p->values[m] = target->values[m];
					p->floorId[m] = target->floorId[m];
				}
			}
		}
	}
}

void SpatialStructure::valuePlus(SpatialStructure &ss) {
	if (
		this->rows != ss.rows ||
		this->cols != ss.cols ||
		this->blx != ss.blx ||
		this->bly != ss.bly ||
		this->dx != ss.dx ||
		this->noValue != ss.noValue ||
		!this->cellArray  ||
		!ss.cellArray
		)
	{
		return;
	}//保证头部信息相同以及矩阵存在
		

	int i, j, m, length;
	SpatialCell *p, *target;
	/*double *p, *target;*/
#pragma omp parallel for private(j,m,length,p,target)
	for (i = 0; i < this->rows; i++) {
		for (j = 0; j < this->cols; j++) {
			p = &this->cellArray[i][j];
			target = &ss.cellArray[i][j];
			/*length = target->nFloor;
			p->nFloor = length;*/
			if ( p->values && 
				target->values && 
				p->nFloor==target->nFloor 
			) 
			{
				length = p->nFloor;
				for (m = 0; m < length; m++) {
					p->values[m] += target->values[m];
				}
			}
		}
	}
}

//释放堆区内存
int SpatialStructure::finalize() {
	int i, j;
	if (!this->cellArray) return -1;
	for (i = 0; i < this->rows; i++) 
	{
		for (j = 0; j < this->cols; j++) 
		{
			this->cellArray[i][j].clear();//释放计算单元内的堆区内存
		}
		delete[] this->cellArray[i]; this->cellArray[i] = NULL;
	}
	delete[] this->cellArray; this->cellArray = NULL;
	return 0;
}

