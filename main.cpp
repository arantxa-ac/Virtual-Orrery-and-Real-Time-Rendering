#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/vector_angle.hpp>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#define STARS 0
#define SUN 1
#define EARTH 2
#define MOON 3

struct Bodies {
	glm::mat4 sun;
	glm::mat4 earth;
	glm::mat4 moon;
};

Bodies bodies;
bool rotate = true;
float lastTime;



void updateGPUGeometry(GPU_Geometry &gpuGeom, CPU_Geometry const &cpuGeom) {
	gpuGeom.bind();
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
	gpuGeom.setNormals(cpuGeom.normals);
	gpuGeom.setTextCoords(cpuGeom.texCoords);
}

// EXAMPLE CALLBACKS
class Assignment4 : public CallbackInterface {

public:
	Assignment4() : camera(0.0, 0.0, 2.0), aspect(1.0f) {
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			rotate = !rotate;
		}
	}
	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS) {
				rightMouseDown = true;
			} else if (action == GLFW_RELEASE) {
				rightMouseDown = false;
			}
		}
	}
	virtual void cursorPosCallback(double xpos, double ypos) {
		if (rightMouseDown) {
			double dx = xpos - mouseOldX;
			double dy = ypos - mouseOldY;
			camera.incrementTheta(dy);
			camera.incrementPhi(dx);
		}
		mouseOldX = xpos;
		mouseOldY = ypos;
	}
	virtual void scrollCallback(double xoffset, double yoffset) {
		camera.incrementR(yoffset);
	}
	virtual void windowSizeCallback(int width, int height) {
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width,  height);
		aspect = float(width)/float(height);
	}

	void viewPipeline(ShaderProgram &sp, int body) {
		glm::mat4 model;
		
		if(body == SUN) {
			bodies.sun = glm::mat4(1.0);
			if (rotate) {
				bodies.sun = glm::rotate(bodies.sun, (float)glfwGetTime() * 0.5f, glm::vec3(0.f, 1.f, 0.f));
				lastTime = glfwGetTime();
			}
			else {
				bodies.sun = glm::rotate(bodies.sun, lastTime * 0.5f, glm::vec3(0.f, 1.f, 0.f));
			}
			model = bodies.sun;
		}
		else if (body == EARTH) {
			float axialTilt = 23.5f;
			float orbRad = 1.f;
			float orbTilt = 0.5f;
			bodies.earth = glm::scale(bodies.sun, glm::vec3(0.5f, 0.5f, 0.5f));
			if (rotate) {
				bodies.earth = glm::rotate(bodies.earth, (float)glfwGetTime() * 2.f, glm::vec3(0.f, 1.f, 0.f));
				bodies.earth = glm::translate(bodies.earth, glm::vec3(cos(glfwGetTime()) * orbRad, cos(glfwGetTime()) * orbTilt, sin(glfwGetTime()) * orbRad));
				bodies.earth = glm::rotate(bodies.earth, glm::radians(axialTilt), glm::vec3(1.f, 0.f, 0.f));
			}
			else {
				bodies.earth = glm::rotate(bodies.earth, lastTime * 2.f, glm::vec3(0.f, 1.f, 0.f));
				bodies.earth = glm::translate(bodies.earth, glm::vec3(cos(lastTime) * orbRad, cos(lastTime) * orbTilt, sin(lastTime) * orbRad));
				bodies.earth = glm::rotate(bodies.earth, glm::radians(axialTilt), glm::vec3(1.f, 0.f, 0.f));
			}
			
			model = bodies.earth;
		}
		else if (body == MOON) {
			float axialTilt = 1.5f;
			float orbRad = 0.7f;
			float orbTilt = 0.5f;
			bodies.moon = glm::scale(bodies.earth, glm::vec3(0.5f, 0.5f, 0.5f));
			if (rotate) {
				bodies.moon = glm::rotate(bodies.moon, (float)glfwGetTime() * 2, glm::vec3(0.f, 1.f, 0.f));
				bodies.moon = glm::translate(bodies.moon, glm::vec3(sin(glfwGetTime() * 2) * orbRad, cos(glfwGetTime() * 4) * orbTilt, cos(glfwGetTime() * 2) * orbRad));
				bodies.moon = glm::rotate(bodies.moon, glm::radians(axialTilt), glm::vec3(1.f, 0.f, 0.f));
				lastTime = glfwGetTime();
			}
			else {
				bodies.moon = glm::rotate(bodies.moon, lastTime * 2, glm::vec3(0.f, 1.f, 0.f));
				bodies.moon = glm::translate(bodies.moon, glm::vec3(sin(lastTime * 2) * orbRad, cos(lastTime * 4) * orbTilt, cos(lastTime * 2) * orbRad));
				bodies.moon = glm::rotate(bodies.moon, glm::radians(axialTilt), glm::vec3(1.f, 0.f, 0.f));
			}

			model = bodies.moon;
		}
		else {
			model = glm::mat4(1.0);
		}

		glm::mat4 M = model;
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);

		GLint location = glGetUniformLocation(sp, "light");
		glm::vec3 light = glm::vec3{ 0.f, 0.f, 1.f };
		glUniform3fv(location, 1, glm::value_ptr(light));

		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

	Camera camera;

private:

	bool rightMouseDown = false;
	float aspect;
	double mouseOldX;
	double mouseOldY;

};

void makeSphere(CPU_Geometry &cpuGeom, float radius) {

	//calculate all points needed for the sphere
	std::vector<std::vector<glm::vec3>> meridians;
	std::vector<std::vector<glm::vec2>> texCoords;
	for (float theta = 0; theta <= glm::pi<float>(); theta += 0.01) {
		std::vector<glm::vec3> points;
		std::vector<glm::vec2> tcs;
		for (float phi = 0; phi <= (2 * glm::pi<float>()); phi += 0.01) {
			points.push_back(glm::vec3{ radius * sin(theta) * cos(phi), -radius * cos(theta), radius * sin(theta) * sin(phi) });
			tcs.push_back(glm::vec2{ phi / (glm::pi<float>() * 2), theta / glm::pi<float>() });
		}
		meridians.push_back(points);
		texCoords.push_back(tcs);
	}
	glm::vec3 southpole = glm::vec3{ radius * sin(glm::pi<float>()) * cos(2 * glm::pi<float>()), -radius * cos(glm::pi<float>()) , radius * sin(glm::pi<float>()) * sin(2 * glm::pi<float>())};
	glm::vec2 southpoleTC = glm::vec2{ 1.f, 1.f };
	//push back the points in appropriate order to the CPU_Geometry
	for (int i = 0; i < (meridians.size()-1); i ++) {
		for (int j = 0; j < (meridians[i].size()-1); j ++) {
			//vertices
			cpuGeom.verts.push_back(meridians[i][j]);
			cpuGeom.verts.push_back(meridians[i + 1][j]);
			cpuGeom.verts.push_back(meridians[i + 1][j + 1]);
			cpuGeom.verts.push_back(meridians[i+1][j+1]);
			cpuGeom.verts.push_back(meridians[i][j + 1]);
			cpuGeom.verts.push_back(meridians[i][j]);

			//texture Coordinates
			cpuGeom.texCoords.push_back(texCoords[i][j]);
			cpuGeom.texCoords.push_back(texCoords[i + 1][j]);
			cpuGeom.texCoords.push_back(texCoords[i + 1][j + 1]);
			cpuGeom.texCoords.push_back(texCoords[i + 1][j + 1]);
			cpuGeom.texCoords.push_back(texCoords[i][j + 1]);
			cpuGeom.texCoords.push_back(texCoords[i][j]);

			//join with the first meridian at the end
			if (j == meridians[i].size() - 2) {
				//vertices
				cpuGeom.verts.push_back(meridians[i][j + 1]);
				cpuGeom.verts.push_back(meridians[i + 1][j+1]);
				cpuGeom.verts.push_back(meridians[i + 1][0]);
				cpuGeom.verts.push_back(meridians[i + 1][0]);
				cpuGeom.verts.push_back(meridians[i][0]);
				cpuGeom.verts.push_back(meridians[i][j + 1]);

				//texture coordinates
				cpuGeom.texCoords.push_back(texCoords[i][j + 1]);
				cpuGeom.texCoords.push_back(texCoords[i + 1][j + 1]);
				cpuGeom.texCoords.push_back(texCoords[i + 1][0]);
				cpuGeom.texCoords.push_back(texCoords[i + 1][0]);
				cpuGeom.texCoords.push_back(texCoords[i][0]);
				cpuGeom.texCoords.push_back(texCoords[i][j + 1]);
			}

			//handle southpole
			if (i == meridians.size() - 2) {
				cpuGeom.verts.push_back(meridians[i + 1][j]);
				cpuGeom.verts.push_back(meridians[i + 1][j + 1]);
				cpuGeom.verts.push_back(southpole);

				cpuGeom.texCoords.push_back(texCoords[i + 1][j]);
				cpuGeom.texCoords.push_back(texCoords[i + 1][j + 1]);
				cpuGeom.texCoords.push_back(southpoleTC);

				if (j == meridians[i].size() - 2) {
					cpuGeom.verts.push_back(meridians[i + 1][j + 1]);
					cpuGeom.verts.push_back(meridians[i + 1][0]);
					cpuGeom.verts.push_back(southpole);

					cpuGeom.texCoords.push_back(texCoords[i + 1][j + 1]);
					cpuGeom.texCoords.push_back(texCoords[i + 1][0]);
					cpuGeom.texCoords.push_back(southpoleTC);
				}
			}
		}
	}

	//add colors and normals
	cpuGeom.cols.resize(cpuGeom.verts.size(), glm::vec3{ 1.0, 0.0, 0.0 });
	cpuGeom.normals.resize(cpuGeom.verts.size(), glm::vec3{ 0.0, 0.0, 0.1 });

	//calculate and add texture coordinates
}


int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired


	GLDebug::enable();

	// CALLBACKS
	auto a4 = std::make_shared<Assignment4>();
	window.setCallbacks(a4);


	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	CPU_Geometry stars;
	makeSphere(stars, 5.f);
	GPU_Geometry starsGPU;
	updateGPUGeometry(starsGPU, stars);
	Texture starstexture("images/stars.jpg", GL_NEAREST);

	CPU_Geometry sun;
	makeSphere(sun, 0.2f);
	GPU_Geometry sunGPU;
	updateGPUGeometry(sunGPU, sun);
	Texture suntexture("images/sun.jpg", GL_NEAREST);

	CPU_Geometry earth;
	makeSphere(earth, 0.2f);
	GPU_Geometry earthGPU;
	updateGPUGeometry(earthGPU, earth);
	Texture earthtexture("images/earth.jpg", GL_NEAREST);

	CPU_Geometry moon;
	makeSphere(moon, 0.2f);
	GPU_Geometry moonGPU;
	updateGPUGeometry(moonGPU, moon);
	Texture moontexture("images/moon.jpg", GL_NEAREST);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		shader.use();

		a4->viewPipeline(shader, STARS);

		updateGPUGeometry(starsGPU, stars);
		starstexture.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(stars.verts.size()));
		starstexture.unbind();

		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		a4->viewPipeline(shader, SUN);

		updateGPUGeometry(sunGPU, sun);
		suntexture.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(sun.verts.size()));
		suntexture.unbind();

		a4->viewPipeline(shader, EARTH);

		updateGPUGeometry(earthGPU, earth);
		earthtexture.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(earth.verts.size()));
		earthtexture.unbind();

		a4->viewPipeline(shader, MOON);

		updateGPUGeometry(moonGPU, moon);
		moontexture.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(moon.verts.size()));
		moontexture.unbind();

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
