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

// definindo valor de pi
#define PI  3.14159265359
// definindo valor de metade de pi
#define HALF_PI PI/2.0

// configurando namespace
using namespace std;

// configurando tamanho da tela
const GLint WIDTH = 1200, HEIGHT = 900;

// gerando vetores que vamos utilizar
// pontos selecionados

vector<glm::vec3*>* pontosSelecionados = new vector<glm::vec3*>();
// a curva inteira original
vector<glm::vec3*>* curvaOriginal = new vector<glm::vec3*>();
// a curva externa gerada
vector<glm::vec3*>* curvaExterna = new vector<glm::vec3*>();
// a curva interna gerada
vector<glm::vec3*>* curvaInterna = new vector<glm::vec3*>();
// pontos finais da curva 
vector<glm::vec3*>* pontosFinais = new vector<glm::vec3*>();
// pontos final em GLfloat
vector<GLfloat>* pontosFinaisFloat = new vector<GLfloat>();

// tamanho da curva interna
int tamanhoCurvaInterna = 0;
// tamanho da curva externa
int tamanhoCurvaExterna = 0;

// quantidade de faces
int faces = 0;

bool draw = false;
GLuint vao, vbo;

// declarando nomes de metodos
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void convertCoordinates(double &x, double &y);
int getZone(float x, float y);
vector<glm::vec3*>* gerarCurva(vector<glm::vec3*>* points);
vector<glm::vec3*>* gerarCurvaExternaInterna(vector<glm::vec3*>* points, bool external);
vector<glm::vec3*>* gerarCurvaFinal(vector<glm::vec3*>* internalCurve, vector<glm::vec3*>* externalCurve);
vector<GLfloat>* convertToFloat(std::vector<glm::vec3*>* points);

int main() {

	////////////////////////////// adicionando configuracoes para a janela

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
	
	// inicializando a janela
	glViewport(0, 0, screenWidth, screenHeight);

	// configurando shader
	Shader coreShader("Shaders/Core/core.vert", "Shaders/Core/core.frag");
	coreShader.Use();
	
	// configurando vao e vbo
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glfwMakeContextCurrent(window);
	// configurando funcao para pegar o click do mouse
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// gerando objeto de writer de mtl
	MTLWriter MTLWriter;
	// criando arquivo para adicionar os pontos mtl
	MTLWriter.createMtlFile();

	// gerando objeto de writer 
	OBJWriter OBJWriter;
	// criando arquivo para adicionar os pontos
	OBJWriter.createOBJFile();

	// while para controle de janela
	while (!glfwWindowShouldClose(window)) {

		// limpando a cor do buffer
		glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
		// gl esperando eventos
		glfwPollEvents();

		// gl esperando botao para fechar a janela
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
		
		// enquanto ainda estiver desenhando
		if (draw == true) {
			// vai dando bind no vao
			glBindVertexArray(vao);
			// e desenhando os triangulos, porem enviando para o vetor de pontos finais

			glDrawArrays(GL_TRIANGLES, 0, pontosFinaisFloat->size());
		}
		// gera o desenho na tela
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return EXIT_SUCCESS;
}

// convertendo de vec3 para GLfloat
std::vector<GLfloat>* convertToFloat(std::vector<glm::vec3*>* points) {
	
std:vector<GLfloat>* temp = new std::vector<GLfloat>();

	for (int i = 0; i < points->size(); i++) {
		temp->push_back(points->at(i)->x);
		temp->push_back(points->at(i)->y);
		temp->push_back(points->at(i)->z);
	}
	return temp;
}

// convertendo coordenadas da tela para coordenadas graficas, x e y para valores entre -1 0 1
void convertCoordinates(double &x, double &y) {

	// se x for maior que a metade da tela da esquerda
	// ou seja clique na direita
	// resultando em valores entre 0 e 1
	if (x > (WIDTH / 2)) {
		x = x - (WIDTH / 2);
		x = x / (WIDTH / 2);
	}
	// se estiver no meio
	else if (x == (WIDTH / 2)) {
		x = 0;
	}
	// se o clique for na parte esquerda da tela, entao converte para valores entre -1 e 0
	else {
		x = -(((WIDTH / 2) - x) / (WIDTH / 2));
	}

	// se y for maior que a metade de baixo da tela
	// ou seja clique na parte de baixo
	// resultando em valores entre 0 e -1
	if (y > (HEIGHT / 2)) {
		y = y - (HEIGHT / 2);
		y = y / (HEIGHT / 2);
		y = y * (-1);
	}
	// se estiver no meio
	else if (y == (HEIGHT / 2)) {
		y = 0;
	}
	// se o clique for na parte de cima da tela, entao converte para valores entre 0 e 1
	else {
		y = -(((HEIGHT / 2) - y) / (HEIGHT / 2));
		y = y * (-1);
	}
}

// pega o quadrante que esta
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

// pegando os cliques da tela
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	
	// se clicar no botao da esquerda do mouse
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		// pega posicao
		glfwGetCursorPos(window, &xpos, &ypos);
		// converte as coordenadas
		convertCoordinates(xpos, ypos);
		
		// gera um novo vec3 com o ponto para a curva
		glm::vec3* point = new glm::vec3(xpos, ypos, 0.0);
		// adiciona ao vetor de pontos selecionados

		pontosSelecionados->push_back(point);

		cout << "- Ponto de Controle Computado:" << endl;
		cout << "x = " << xpos << endl;
		cout << "y = " << ypos << endl;		

		// arrendodamento de curva, aumentando um pouco a curva
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
	// quando clicar no botao direito, terminar e finalizar a curva
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		draw = true;

		curvaOriginal = gerarCurva(pontosSelecionados);
		curvaExterna = gerarCurvaExternaInterna(curvaOriginal, true);
		curvaInterna = gerarCurvaExternaInterna(curvaOriginal, false);

		// tamanho do arrya dividido por 2 - porque a metade desses valores e cor branca

		tamanhoCurvaExterna = curvaExterna->size() / 2.0;
		tamanhoCurvaInterna = curvaInterna->size() / 2.0;

		OBJWriter OBJWriter;
		OBJWriter.saveTextureValuesToOBJ();

		pontosFinais = gerarCurvaFinal(curvaInterna, curvaExterna);

		pontosFinaisFloat = convertToFloat(pontosFinais);
	
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*pontosFinaisFloat->size(), &pontosFinaisFloat->at(0), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	}
}

// gera a curva (pontos) de um ponto ao outro

vector<glm::vec3*>* gerarCurva(vector<glm::vec3*>* points) {

	// cria o txt
	TXTWriter TXTWriter;
	TXTWriter.createTXTFile();

	vector<glm::vec3*>* curvaCalculada = new vector<glm::vec3*>();
	vector<glm::vec3*>* temp = new vector<glm::vec3*>();
	vector<glm::vec3*>* temp2 = new vector<glm::vec3*>();

	for (int i = 0; i < points->size(); i++) {
		temp->push_back(new glm::vec3(points->at(i)->x, points->at(i)->y, 0));
	}

	//cria mais um ponto para terminar a curva, com o ponto inicial
	temp->push_back(points->at(0));
	temp->push_back(points->at(1));
	temp->push_back(points->at(2));

	// itera entre os pontos coletados
	for (int i = 0; i < (temp->size() - 3); i++) { // sem utilizar o ultimo ponto

		// itera entre 99 variacoes para a distancia entre cada ponto
		for (int j = 0; j<100; ++j){

			// todos estess valores vao dar dizima periodica
			float t = static_cast<float>(j)/99.0;
		
			// calculando bspline para x
			GLfloat x = (((-1 * pow(t, 3) + 3 * pow(t, 2) - 3 * t + 1)*temp->at(i)->x +
				(3 * pow(t, 3) - 6 * pow(t, 2) + 0 * t + 4)*temp->at(i+1)->x +
				(-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1)*temp->at(i+2)->x +
				(1 * pow(t, 3) + 0 * pow(t, 2) + 0 * t + 0)*temp->at(i+3)->x) / 6);

			// calculando bspline para y
			GLfloat y = (((-1 * pow(t, 3) + 3 * pow(t, 2) - 3 * t + 1)*temp->at(i)->y +
				(3 * pow(t, 3) - 6 * pow(t, 2) + 0 * t + 4)*temp->at(i+1)->y +
				(-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1)*temp->at(i+2)->y +
				(1 * pow(t, 3) + 0 * pow(t, 2) + 0 * t + 0)*temp->at(i+3)->y) / 6);
			
			// depois da conversao de x e y coloca em uma vec3
			glm::vec3* point = new glm::vec3(x, y, 0.0);
			// adiciona o ponto no vetor de curvas calculadas

			curvaCalculada->push_back(point);

			// adiciona o ponto no txt
			TXTWriter.addPoint(point->x, point->y, point->z);

			// adiciona cor branca para a curva

			curvaCalculada->push_back(new glm::vec3(1.0, 0.0, 1.0));

		}	
	}
	// termina o arquivo txt
	TXTWriter.closeTXTFile();
	cout << "Curva gerada com sucesso!" << endl;
	// retorna um vec3 com os pontos da curva

	return curvaCalculada;
}

vector<glm::vec3*>* gerarCurvaExternaInterna(vector<glm::vec3*>* points, bool external) {

	// recebe os pontos da curva original do meio
	
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

		// arco tangente
		GLfloat angle = glm::atan(dy, dx);

		// verifica se gera a curva interna ou a externa
		if (external) {
			angle += HALF_PI;
		}
		else {
			angle -= HALF_PI;
		}

		// 0.09 -> tamanho da curva, fator de escala
		GLfloat offsetX = glm::cos(angle) * 0.09;
		GLfloat offsetY = glm::sin(angle) * 0.09;
		
		// pronto da curva principal + escala

		glm::vec3* pontosGerados = new glm::vec3(a->x + offsetX, a->y + offsetY, 0.0);

		calculatedCurve->push_back(pontosGerados);

		// adiciona pro obj
		OBJWriter.addPointsFinalCurve(pontosGerados->x, pontosGerados->y, pontosGerados->z);
		
		// adiciona cor branca para curva
		calculatedCurve->push_back(new glm::vec3(1.0, 0.0, 1.0)); 

	}
	return calculatedCurve;
}

vector<glm::vec3*>* gerarCurvaFinal(vector<glm::vec3*>* internalCurve, vector<glm::vec3*>* externalCurve) {
	OBJWriter OBJWriter;

	int i = 0;
	int index = 1;

	for (; i < internalCurve->size() - 2; i += 2) {
		// Ponto Interno 1
		pontosFinais->push_back(internalCurve->at(i));
		pontosFinais->push_back(internalCurve->at(i + 1));

		glm::vec3* a_int = internalCurve->at(i);

		// Ponto Interno 2
		pontosFinais->push_back(internalCurve->at(i + 2));
		pontosFinais->push_back(internalCurve->at(i + 3));

		glm::vec3* b_int = internalCurve->at(i + 2);

		// Ponto Externo 1
		pontosFinais->push_back(externalCurve->at(i));
		pontosFinais->push_back(externalCurve->at(i + 1));

		glm::vec3* c_ext = externalCurve->at(i);
		
		OBJWriter.addFaces(index, tamanhoCurvaExterna, ++faces, 1);

		// Ponto Interno 2
		pontosFinais->push_back(internalCurve->at(i + 2));
		pontosFinais->push_back(internalCurve->at(i + 3));

		// Ponto Externo 2
		pontosFinais->push_back(externalCurve->at(i + 2));
		pontosFinais->push_back(externalCurve->at(i + 3));

		glm::vec3* d_ext = externalCurve->at(i + 2);

		// Ponto Externo 1
		pontosFinais->push_back(externalCurve->at(i));
		pontosFinais->push_back(externalCurve->at(i + 1));

		OBJWriter.addFaces(index, tamanhoCurvaExterna, ++faces, 2);

		// pega os vetores das normais
		// y e z sao invertidos para modificar os eixos
		// produto escalar
		glm::vec3 ab = glm::vec3(b_int->x - a_int->x, b_int->z - a_int->z, b_int->y - a_int->y);
		glm::vec3 ac = glm::vec3(c_ext->x - a_int->x, c_ext->z - a_int->z, c_ext->y - a_int->y);
		glm::vec3 dc = glm::vec3(c_ext->x - d_ext->x, c_ext->z - d_ext->z, c_ext->y - d_ext->y);
		glm::vec3 db = glm::vec3(b_int->x - d_ext->x, b_int->z - d_ext->z, b_int->y - d_ext->y);

		// prooduto vetorial
		glm::vec3 normal_vec_abac = glm::cross(ac, ab);
		glm::vec3 normal_vec_dbdc = glm::cross(db, dc);

		OBJWriter.addNormalExternalCurve(normal_vec_abac, normal_vec_dbdc);
		
		index++;
	}
	cout << i << " , " << index << endl;
	// O trecho abaixo liga os últimos pontos com primeiro os primeiros
	// Ponto Interno 1
	pontosFinais->push_back(internalCurve->at(i));
	pontosFinais->push_back(internalCurve->at(i + 1));

	glm::vec3* a_int = internalCurve->at(i);

	// Ponto Interno 2
	pontosFinais->push_back(internalCurve->at(0));
	pontosFinais->push_back(internalCurve->at(1));

	glm::vec3* b_int = internalCurve->at(0);

	// Ponto Externo 1
	pontosFinais->push_back(externalCurve->at(i));
	pontosFinais->push_back(externalCurve->at(i + 1));

	glm::vec3* c_ext = externalCurve->at(i);
	
	OBJWriter.addFaces(index, tamanhoCurvaExterna, ++faces, 3);

	// Ponto Interno 2
	pontosFinais->push_back(internalCurve->at(0));
	pontosFinais->push_back(internalCurve->at(1));

	// Ponto Externo 2
	pontosFinais->push_back(externalCurve->at(0));
	pontosFinais->push_back(externalCurve->at(1));

	glm::vec3* d_ext = externalCurve->at(0);

	// Ponto Externo 1
	pontosFinais->push_back(externalCurve->at(i));
	pontosFinais->push_back(externalCurve->at(i + 1));

	OBJWriter.addFaces(index, tamanhoCurvaExterna, ++faces, 4);

	// pega os vetores das normais
	// y e z sao invertidos para modificar os eixos
	// produto escalar
	glm::vec3 ab = glm::vec3(a_int->x - b_int->x, a_int->z - b_int->z, a_int->y - b_int->y);
	glm::vec3 ac = glm::vec3(a_int->x - c_ext->x, a_int->z - c_ext->z, a_int->y - c_ext->y);
	glm::vec3 dc = glm::vec3(d_ext->x - c_ext->x, d_ext->z - c_ext->z, d_ext->y - c_ext->y);
	glm::vec3 db = glm::vec3(d_ext->x - b_int->x, d_ext->z - b_int->z, d_ext->y - b_int->y);

	// prooduto vetorial
	glm::vec3 normal_vec_abac = glm::cross(ab, ac);
	glm::vec3 normal_vec_dbdc = glm::cross(db, dc);

	OBJWriter.addNormalExternalCurve(normal_vec_abac, normal_vec_dbdc);

	return pontosFinais;
}

