#include "global.h"
#include "couple.h"
#include "tools.h"
#include <iomanip>

using namespace std;

void saveData() {
	cout << "write once:" << modelControl.currentSimTime << "s" << endl;
	write_SpatialStructure(parFile.saveDir, parFile.resultRoot, ".wd", modelControl.saveIndex, domain.waterDepths, parFile.sectionCount);

	if (parFile.monitor && ModelMonitor.points) //输出监测点水深
	{
		ModelMonitor.writePoints();
	}

	if (modelControl.timeToEndSimulation()) //在模拟结束时输出最大水深或最大水位 
	{

		if (parFile.maxH) //输出最大水深
		{
			write_SpatialStructure(parFile.saveDir, "maxH", ".asc", -1, domain.maxH, parFile.sectionCount);
		}

		if (parFile.maxElev && parFile.maxH) //输出最大水位
		{
			domain.maxElev.clone(domain.elevModel);
			domain.maxElev.valuePlus(domain.maxH);
			write_SpatialStructure(parFile.saveDir, "maxWaterElev", ".asc", -1, domain.maxElev, parFile.sectionCount);
		}
		if (parFile.reportNode) {
			reportNodeFlood(parFile.nodeReportPath.c_str());
		}
	}
}

void write_SpatialStructure(string saveDir, string resultRoot, string extension, int saveIndex, SpatialStructure &ss, int sectionNum) {
	int i, index;
	double temp;
	double noval = ss.noValue;
	char fnam[300];
	/*string saveIndexS = saveIndex >= 0 ? to_string(saveIndex) : "";*/
	/*char tmp_sys_com[255];*/
	ofstream sections[MAXSECTIONS];
	SpatialCell *cell = NULL;
	saveDir = saveDir.substr(0, 1 + saveDir.find_last_not_of("\\"));
	for (i = 1; i <= sectionNum; i++)
	{
		if (saveIndex >= 0 && saveIndex <= 9999)	sprintf_s(fnam, sizeof(fnam), "%s\\section#%d\\%s-%.4d%s", saveDir.c_str(), i, resultRoot.c_str(), saveIndex, extension.c_str());
		else if (saveIndex > 9999)				sprintf_s(fnam, sizeof(fnam), "%s\\section#%d\\%s-%d%s", saveDir.c_str(), i, resultRoot.c_str(), saveIndex, extension.c_str());
		else if (saveIndex == -1) {
			sprintf_s(fnam, sizeof(fnam), "%s\\section#%d\\%s%s", saveDir.c_str(), i, resultRoot.c_str(), extension.c_str());
		}
		else { 
			cout << "saveIndex wrong" << endl;
			return;
		}
		sections[i].open(fnam, ios::out);
		if (!sections[i].is_open())
		{
			cout << "Problems writing to file (when writing SpatialStructure): " << fnam << endl;
			/*printf("Problems writing to file %s\n", fnam);*/
			return;
		}
	}
	for (i = 1; i <= sectionNum; i++)
	{
		sections[i] << "ncols         "  << ss.cols << "\n";
		sections[i] << "nrows         " << ss.rows << "\n";
		sections[i] << "xllcorner         " <<  setiosflags(ios::fixed) << setprecision(4)<<ss.blx << "\n";
		sections[i] << "yllcorner         " << setiosflags(ios::fixed) << setprecision(4) <<ss.bly << "\n";
		sections[i] << "cellsize         " << ss.dx << "\n";
		sections[i] << "NODATA_value         " << ss.noValue << "\n";
		for (int m = 0; m < ss.rows; m++) {
			cell = ss.cellArray[m];
			for (int n = 0; n < ss.cols; n++) {
				index = findIndexInArray("#" + to_string(i), cell->floorId, cell->nFloor);
				if (index == -1)
				{
					sections[i] << noval<<"\t";
				}
				else
				{
					sections[i] << cell->values[index]<<"\t";
				}
				cell++;
			}
			sections[i] << "\n";
		}
		sections[i].close();
	}

}