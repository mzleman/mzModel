#include "mzModel.h"
using namespace std;

SpatialCell::SpatialCell() {
	this->nFloor = 0;
	this->floorId = NULL;
	this->values = NULL;
	this->eastCell = NULL;
	this->southCell = NULL;
	this->westCell = NULL;
	this->northCell = NULL;
}

void SpatialCell::addValue(double val,string id) {
	if (this->nFloor == 0) {
		++this->nFloor;
		this->values = new double[1];
		this->values[0] = val;
		this->floorId = new string[1];
		this->floorId[0] = id;
	}
	else {
		++this->nFloor; 
		double* temp = new double[this->nFloor];
		string* tempstr = new string[this->nFloor];
		int i;
		for ( i = 0; i < this->nFloor-1; ++i) {
			temp[i] = this->values[i];
			tempstr[i] = this->floorId[i];
		}
		temp[i] = val;
		tempstr[i] = id;
		delete[] this->values;
		delete[] this->floorId;
		this->values = temp;
		this->floorId = tempstr;
	}
}

int SpatialCell::clear() {
	if (this->values) {
		delete[] this->values; this->values = NULL;
	}
	if (this->floorId) {
		delete[] this->floorId; this->floorId = NULL;
	}
	return 0;
}