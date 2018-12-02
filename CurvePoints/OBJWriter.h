#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#ifndef OBJWRITER_H
#define OBJWRITER_H

using namespace std;

class OBJWriter
{
public:
	ofstream OBJFile;

	OBJWriter() {}
	void createOBJFile() {
		OBJFile.open("curve.obj");
		OBJFile << "mtllib " << "curve.mtl" << "\n" << endl;
		OBJFile << "g " << "road" << "\n" << endl;
		OBJFile << "usemtl road\n" << endl;
		OBJFile.close();
	}

	void addFaces(int index, int externalCurveSize, int faces, int n) {
		OBJFile.open("curve.obj", ios::app);
		if (n == 1) {
			OBJFile << "f " << index + externalCurveSize << "/" << 1 << "/" << faces << " " <<
				index + 1 + externalCurveSize << "/" << 2 << "/" << faces << " " <<
				index << "/" << 3 << "/" << faces << endl;
		}
		if (n == 2) {
			OBJFile << "f " << index + 1 + externalCurveSize << "/" << 2 << "/" << faces << " " <<
				index + 1 << "/" << 4 << "/" << faces << " " <<
				index << "/" << 3 << "/" << faces << endl;
		}
		if (n == 3) {
			OBJFile << "f " << index + externalCurveSize << "/" << 1 << "/" << faces << " " <<
				externalCurveSize + 1 << "/" << 2 << "/" << faces << " " <<
				index << "/" << 3 << "/" << faces << endl;
		}
		if (n == 4) {
			OBJFile << "f " << 1 + externalCurveSize << "/" << 2 << "/" << faces << " " <<
				1 << "/" << 4 << "/" << faces << " " <<
				index << "/" << 3 << "/" << faces << endl;
		}
		OBJFile.close();
	}

	void addNormalExternalCurve(glm::vec3 normal_vec_abac, glm::vec3 normal_vec_dbdc) {
		OBJFile.open("curve.obj", ios::app);
		OBJFile << "vn " << normal_vec_abac[0] << " " << normal_vec_abac[1] << " " << normal_vec_abac[2] << endl;
		OBJFile << "vn " << normal_vec_dbdc[0] << " " << normal_vec_dbdc[1] << " " << normal_vec_dbdc[2] << endl;
		OBJFile.close();
	}

	void addPointsFinalCurve(float x, float y, float z) {
		OBJFile.open("curve.obj", ios::app);
		OBJFile << "v " << x << " " << z << " " << y << endl;
		OBJFile.close();
	}
	
	void saveTextureValuesToOBJ() {
		//save texture points on obj
		OBJFile.open("curve.obj", ios::app);
		OBJFile << endl;
		OBJFile << "vt " << 1.0 << " " << 1.0 << endl; //-1.0 is to treat the 1 - v on the objreader
		OBJFile << "vt " << 1.0 << " " << 0.0 << endl; //+1.0 is to treat the 1 - v on the objreader
		OBJFile << "vt " << 0.0 << " " << 1.0 << endl;
		OBJFile << "vt " << 0.0 << " " << 0.0 << endl;
		OBJFile << endl;
		OBJFile.close();
	}
};

#endif