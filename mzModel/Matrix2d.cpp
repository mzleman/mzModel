#include "mzModel.h"
#include <iomanip>
using namespace std;

Matrix2d::Matrix2d(string path) {
	ifstream arrfile(path);
	int start = path.rfind("#");
	int end = path.rfind(".");
	if (!arrfile.is_open() || start == path.npos || end == path.npos|| start>=end)
	{
		std::cout << path << " open fail" << endl;
		this->cols=this->rows = 0;
		this->blx = this->bly = this->dx =this->noValue= 0.0;
		this->id = this->fileName=this->fileType="";
		this->arr = NULL;
		return; 
	}
	this->id = path.substr(start, end - start);
	this->fileType = path;
	string temp;
	arrfile >> temp >> this->cols;
	arrfile >> temp >> this->rows;
	arrfile >> temp >> this->blx;
	arrfile >> temp >> this->bly;
	arrfile >> temp >> this->dx;
	arrfile >> temp >> this->noValue;
	cout << "Matrix data:  ";
	cout << this->rows << "  " << this->cols << "  ";
	cout << setprecision(12) << this->blx <<"  "<< this->bly << "  " << this->dx << "  " << this->noValue << endl;
	this->arr = new double*[this->rows];
	int i, j;
	for (i = 0; i < this->rows; i++) {
		this->arr[i] = new double[this->cols];
		for (j = 0; j < this->cols; j++)
			arrfile >> this->arr[i][j];
	}
	arrfile.close();
}

Matrix2d::~Matrix2d() {
	if (this->arr)
	{
		int i, j;
		for (i = 0; i < this->rows; i++) {
			delete[] this->arr[i];
		}
		delete[] this->arr;
	}
	cout << this->fileType <<" id:"<< this->id << " ÄÚ´æÊÍ·Å" << endl;
}