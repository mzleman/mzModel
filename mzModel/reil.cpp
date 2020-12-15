#include "mzModel.h"
using namespace std;

//Rain Evep Infil Loss


int Rainfall::count = 0;
Rainfall::Rainfall() {
	this->startTime = this->endTime = this->index = 0;
	this->rain_intensity = 0;
	this->radarRain = NULL;
}

int Rainfall::clear() {
	if (this->radarRain) {
		/*------------------------------------

		根据雷达降雨的数据类型设计相应的清除方法

		-------------------------------------*/
	}
	return 0;
}

Loss::Loss() {
	this->cols = this->rows = 0;
	this->loss_rate = 0;
	this->cellArr = NULL;
}

int Loss::clear() {
	if (this->cellArr) {
		for (int i = 0; i < this->rows; i++) {
			for (int j = 0; j < this->cols; j++) {
				if (this->cellArr[i][j].values) {
					delete[] this->cellArr[i][j].values; this->cellArr[i][j].values = NULL;
				}
			}
			delete[] this->cellArr[i]; this->cellArr = NULL;
		}
		delete[] this->cellArr;
	}
	return 0;
}

Evaporation::Evaporation(){
	this->cols = this->rows = 0;
	this->index = 0;
	this->startTime = this->endTime = 0;
	this->evap_rate = 0.0;
	this->cellArr = NULL;
}

int Evaporation::clear() {
	if (this->cellArr) {
		for (int i = 0; i < this->rows; i++) {
			for (int j = 0; j < this->cols; j++) {
				if (this->cellArr[i][j].values) {
					delete[] this->cellArr[i][j].values; this->cellArr[i][j].values = NULL;
				}
			}
			delete[] this->cellArr[i]; this->cellArr = NULL;
		}
		delete[] this->cellArr;
	}
	return 0;
}