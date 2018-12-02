#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
#include <vec2.hpp>
#include "Shader.h"
#include "MTLWriter.h"
#include "TXTWriter.h"
#include "OBJWriter.h"

#define PI  3.14159265359
#define HALF_PI PI/2.0

using namespace std;

const GLint WIDTH = 1200, HEIGHT = 900;

vector<glm::vec3*>* selectedPoints = new vector<glm::vec3*>();
vector<glm::vec3*>* originalCurve = new vector<glm::vec3*>();
vector<glm::vec3*>* externalCurve = new vector<glm::vec3*>();
vector<glm::vec3*>* internalCurve = new vector<glm::vec3*>();
vector<glm::vec3*>* finalPoints = new vector<glm::vec3*>();
vector<GLfloat>* finalPointsFloat = new vector<GLfloat>();

int internalCurveSize = 0;
int externalCurveSize = 0;

int faces = 0;

bool draw = false;
GLuint vao, vbo;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void convertCoordinates(double &x, double &y);
int getZone(float x, float y);
vector<glm::vec3*>* generateCurve(vector<glm::vec3*>* points);
vector<glm::vec3*>* generateExternalCurve(vector<glm::vec3*>* points, bool external);
vector<glm::vec3*>* generateFinalCurve(vector<glm::vec3*>* internalCurve, vector<glm::vec3*>* externalCurve);
vector<GLfloat>* convertToFloat(std::vector<glm::vec3*>* points);

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Curve generator", nullptr, nullptr);

	int screenWidth, screenHeight;
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

	if (nullptr == window) {
		std::cout << "Falha ao criar janela GLFW" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;

	if (GLEW_OK != glewInit()) {
		std::cout << "Falha ao criar janela GLEW" << std::endl;
		return EXIT_FAILURE;
	}
	
	glViewport(0, 0, screenWidth, screenHeight);

	Shader coreShader("Shaders/Core/core.vert", "Shaders/Core/core.frag");
	coreShader.Use();
		
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glfwMakeContextCurrent(window);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	MTLWriter MTLWriter;
	MTLWriter.createMtlFile();

	OBJWriter OBJWriter;
	OBJWriter.createOBJFile();

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
				
		if (draw == true) {
			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, finalPointsFloat->size());
		}		

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return EXIT_SUCCESS;
}

std::vector<GLfloat>* convertToFloat(std::vector<glm::vec3*>* points) {
	//convert from vec3 to GLfloat
std:vector<GLfloat>* temp = new std::vector<GLfloat>();

	for (int i = 0; i < points->size(); i++) {
		temp->push_back(points->at(i)->x);
		temp->push_back(points->at(i)->y);
		temp->push_back(points->at(i)->z);
	}
	return temp;
}

void convertCoordinates(double &x, double &y) {
	//convert resolution coordinates to graph coordinates
	if (x > (WIDTH / 2)) {
		x = x - (WIDTH / 2);
		x = x / (WIDTH / 2);
	}
	else if (x == (WIDTH / 2)) {
		x = 0;
	}
	else {
		x = -(((WIDTH / 2) - x) / (WIDTH / 2));
	}

	if (y > (HEIGHT / 2)) {
		y = y - (HEIGHT / 2);
		y = y / (HEIGHT / 2);
		y = y * (-1);
	}
	else if (y == (HEIGHT / 2)) {
		y = 0;
	}
	else {
		y = -(((HEIGHT / 2) - y) / (HEIGHT / 2));
		y = y * (-1);
	}
}

int getZone(float x, float y) {
	if (x > 0.0 && y > 0.0) {
		return 1;
	}
	else if (x > 0.0 && y < 0.0) {
		return 4;
	}
	else if (x < 0.0 && y < 0.0) {
		return 3;
	}
	else {
		return 2;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		convertCoordinates(xpos, ypos);
		
		glm::vec3* point = new glm::vec3(xpos, ypos, 0.0);
		selectedPoints->push_back(point);
		cout << "ponto registrado" << endl;
		cout << "x = " << xpos << endl;
		cout << "y = " << ypos << endl;		

		int zone = getZone(xpos, ypos);
		if (zone == 1) {
			xpos += 0.5;
			ypos += 0.5;
		}
		else if (zone == 2) {
			xpos -= 0.5;
			ypos += 0.5;
		}
		else if (zone == 3) {
			xpos -= 0.5;
			ypos -= 0.5;
		}
		else if (zone == 4) {
			xpos += 0.5;
			ypos -= 0.5;
		}
	}	
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		draw = true;

		originalCurve = generateCurve(selectedPoints);
		externalCurve = generateExternalCurve(originalCurve, true);
		internalCurve = generateExternalCurve(originalCurve, false);

		externalCurveSize = externalCurve->size() / 2.0;
		internalCurveSize = internalCurve->size() / 2.0;
		
		OBJWriter OBJWriter;
		OBJWriter.saveTextureValuesToOBJ();

		finalPoints = generateFinalCurve(internalCurve, externalCurve);

		finalPointsFloat = convertToFloat(finalPoints);
	
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*finalPointsFloat->size(), &finalPointsFloat->at(0), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	}
}

vector<glm::vec3*>* generateCurve(vector<glm::vec3*>* points) {
	TXTWriter TXTWriter;
	TXTWriter.createTXTFile();

	vector<glm::vec3*>* calculatedCurve = new vector<glm::vec3*>();
	vector<glm::vec3*>* temp = new vector<glm::vec3*>();
	vector<glm::vec3*>* temp2 = new vector<glm::vec3*>();

	for (int i = 0; i < points->size(); i++) {
		temp->push_back(new glm::vec3(points->at(i)->x, points->at(i)->y, 0));
	}

	//close the curve
	temp->push_back(points->at(0));
	temp->push_back(points->at(1));
	temp->push_back(points->at(2));

	//iterate through the collected points
	for (int i = 0; i < (temp->size() - 3); i++) {

		for (int j = 0; j<100; ++j){

			float t = static_cast<float>(j)/99.0;
		
			GLfloat x = (((-1 * pow(t, 3) + 3 * pow(t, 2) - 3 * t + 1)*temp->at(i)->x +
				(3 * pow(t, 3) - 6 * pow(t, 2) + 0 * t + 4)*temp->at(i+1)->x +
				(-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1)*temp->at(i+2)->x +
				(1 * pow(t, 3) + 0 * pow(t, 2) + 0 * t + 0)*temp->at(i+3)->x) / 6);

			GLfloat y = (((-1 * pow(t, 3) + 3 * pow(t, 2) - 3 * t + 1)*temp->at(i)->y +
				(3 * pow(t, 3) - 6 * pow(t, 2) + 0 * t + 4)*temp->at(i+1)->y +
				(-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1)*temp->at(i+2)->y +
				(1 * pow(t, 3) + 0 * pow(t, 2) + 0 * t + 0)*temp->at(i+3)->y) / 6);
					
			glm::vec3* point = new glm::vec3(x, y, 0.0);
			calculatedCurve->push_back(point);
			TXTWriter.addPoint(point->x, point->y, point->z);

			calculatedCurve->push_back(new glm::vec3(1.0, 1.0, 1.0));
		}	
	}
	TXTWriter.closeTXTFile();
	cout << "Curva gerada com sucesso" << endl;
	return calculatedCurve;
}

vector<glm::vec3*>* generateExternalCurve(vector<glm::vec3*>* points, bool external) {
	OBJWriter OBJWriter;
	vector<glm::vec3*>* calculatedCurve = new vector<glm::vec3*>();

	for (int j = 0; j < points->size() - 1; j += 2) {

		glm::vec3* a = points->at(j);
		glm::vec3* b;

		if (j == points->size() - 2) {
			b = points->at(0);
		}
		else {
			b = points->at(j + 2);
		}

		GLfloat dx = b->x - a->x;
		GLfloat dy = b->y - a->y;

		if (dx == 0 || dy == 0) {
			dx = b->x - points->at(j - 2)->x;
			dy = b->y - points->at(j - 2)->y;
		}

		GLfloat angle = glm::atan(dy, dx);

		if (external) {
			angle += HALF_PI;
		}
		else {
			angle -= HALF_PI;
		}

		GLfloat offsetX = glm::cos(angle) * 0.09;
		GLfloat offsetY = glm::sin(angle) * 0.09;
		
		glm::vec3* pointGenerated = new glm::vec3(a->x + offsetX, a->y + offsetY, 0.0);
		calculatedCurve->push_back(pointGenerated);
		OBJWriter.addPointsFinalCurve(pointGenerated->x, pointGenerated->y, pointGenerated->z);

		calculatedCurve->push_back(new glm::vec3(1.0, 1.0, 1.0)); 
	}
	return calculatedCurve;
}

vector<glm::vec3*>* generateFinalCurve(vector<glm::vec3*>* internalCurve, vector<glm::vec3*>* externalCurve) {
	OBJWriter OBJWriter;

	int i = 0;
	int index = 1;

	for (; i < internalCurve->size() - 2; i += 2) {
		// Ponto Interno 1
		finalPoints->push_back(internalCurve->at(i));
		finalPoints->push_back(internalCurve->at(i + 1));

		glm::vec3* a_int = internalCurve->at(i);

		// Ponto Interno 2
		finalPoints->push_back(internalCurve->at(i + 2));
		finalPoints->push_back(internalCurve->at(i + 3));

		glm::vec3* b_int = internalCurve->at(i + 2);

		// Ponto Externo 1
		finalPoints->push_back(externalCurve->at(i));
		finalPoints->push_back(externalCurve->at(i + 1));

		glm::vec3* c_ext = externalCurve->at(i);
		
		OBJWriter.addFaces(index, externalCurveSize, ++faces, 1);

		// Ponto Interno 2
		finalPoints->push_back(internalCurve->at(i + 2));
		finalPoints->push_back(internalCurve->at(i + 3));

		// Ponto Externo 2
		finalPoints->push_back(externalCurve->at(i + 2));
		finalPoints->push_back(externalCurve->at(i + 3));

		glm::vec3* d_ext = externalCurve->at(i + 2);

		// Ponto Externo 1
		finalPoints->push_back(externalCurve->at(i));
		finalPoints->push_back(externalCurve->at(i + 1));

		OBJWriter.addFaces(index, externalCurveSize, ++faces, 2);

		//get vectors for the normals
		//y and z are inversed to modify axis
		glm::vec3 ab = glm::vec3(b_int->x - a_int->x, b_int->z - a_int->z, b_int->y - a_int->y);
		glm::vec3 ac = glm::vec3(c_ext->x - a_int->x, c_ext->z - a_int->z, c_ext->y - a_int->y);
		glm::vec3 dc = glm::vec3(c_ext->x - d_ext->x, c_ext->z - d_ext->z, c_ext->y - d_ext->y);
		glm::vec3 db = glm::vec3(b_int->x - d_ext->x, b_int->z - d_ext->z, b_int->y - d_ext->y);

		glm::vec3 normal_vec_abac = glm::cross(ac, ab);
		glm::vec3 normal_vec_dbdc = glm::cross(db, dc);

		OBJWriter.addNormalExternalCurve(normal_vec_abac, normal_vec_dbdc);
		
		index++;
	}
	cout << i << " , " << index << endl;
	// O trecho abaixo liga os últimos pontos com primeiro os primeiros
	// Ponto Interno 1
	finalPoints->push_back(internalCurve->at(i));
	finalPoints->push_back(internalCurve->at(i + 1));

	glm::vec3* a_int = internalCurve->at(i);

	// Ponto Interno 2
	finalPoints->push_back(internalCurve->at(0));
	finalPoints->push_back(internalCurve->at(1));

	glm::vec3* b_int = internalCurve->at(0);

	// Ponto Externo 1
	finalPoints->push_back(externalCurve->at(i));
	finalPoints->push_back(externalCurve->at(i + 1));

	glm::vec3* c_ext = externalCurve->at(i);
	
	OBJWriter.addFaces(index, externalCurveSize, ++faces, 3);

	// Ponto Interno 2
	finalPoints->push_back(internalCurve->at(0));
	finalPoints->push_back(internalCurve->at(1));

	// Ponto Externo 2
	finalPoints->push_back(externalCurve->at(0));
	finalPoints->push_back(externalCurve->at(1));

	glm::vec3* d_ext = externalCurve->at(0);

	// Ponto Externo 1
	finalPoints->push_back(externalCurve->at(i));
	finalPoints->push_back(externalCurve->at(i + 1));

	OBJWriter.addFaces(index, externalCurveSize, ++faces, 4);
	//get vectors for the normals
	//y and z are inversed to modify axis
	glm::vec3 ab = glm::vec3(a_int->x - b_int->x, a_int->z - b_int->z, a_int->y - b_int->y);
	glm::vec3 ac = glm::vec3(a_int->x - c_ext->x, a_int->z - c_ext->z, a_int->y - c_ext->y);
	glm::vec3 dc = glm::vec3(d_ext->x - c_ext->x, d_ext->z - c_ext->z, d_ext->y - c_ext->y);
	glm::vec3 db = glm::vec3(d_ext->x - b_int->x, d_ext->z - b_int->z, d_ext->y - b_int->y);

	glm::vec3 normal_vec_abac = glm::cross(ab, ac);
	glm::vec3 normal_vec_dbdc = glm::cross(db, dc);

	OBJWriter.addNormalExternalCurve(normal_vec_abac, normal_vec_dbdc);

	return finalPoints;
}

