#define _USE_MATH_DEFINES
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

struct GameTexture {
	GameTexture(std::string path, GLenum interpolation) :
		textures(path, interpolation)
	{}

	//CPU_Geometry cgeom;
	//GPU_Geometry ggeom;
	Texture textures;
};

CPU_Geometry shipGeom(float width, float height) {
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;
	CPU_Geometry retGeom;
	// vertices for the spaceship quad
	retGeom.verts.push_back(glm::vec3(-halfWidth, halfHeight, 0.f));
	retGeom.verts.push_back(glm::vec3(-halfWidth, -halfHeight, 0.f));
	retGeom.verts.push_back(glm::vec3(halfWidth, -halfHeight, 0.f));
	retGeom.verts.push_back(glm::vec3(-halfWidth, halfHeight, 0.f));
	retGeom.verts.push_back(glm::vec3(halfWidth, -halfHeight, 0.f));
	retGeom.verts.push_back(glm::vec3(halfWidth, halfHeight, 0.f));

	// For full marks (Part IV), you'll need to use the following vertex coordinates instead.
	// Then, you'd get the correct scale/translation/rotation by passing in uniforms into
	// the vertex shader.

	/*retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, 1.f, 0.f));*/


	// texture coordinates
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 1.f));
	return retGeom;
}

glm::mat4 identityMatrix() {
	return glm::mat4( //glm does it weird where you need to put it as the transpose of the matrix
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

glm::mat4 translationMatrix(float x, float y) {
	return glm::mat4( //glm does it weird where you need to put it as the transpose of the matrix
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		x, y, 0.0f, 1.0f
	);
}

glm::mat4 scalingMatrix(float s) {
	return glm::mat4( //glm does it weird where you need to put it as the transpose of the matrix
		s, 0.0f, 0.0f, 0.0f,
		0.0f, s, 0.0f, 0.0f,
		0.0f, 0.0f, s, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

glm::mat4 rotationMatrix(float beta) {
	//std::cout << "Our result: " << beta << std::endl;
	beta = glm::radians(beta);
	//std::cout << "Our result: " << beta << std::endl;
	return glm::mat4( //clockwise
		std::cos(beta), std::sin(beta), 0.0f, 0.0f,
		-std::sin(beta), std::cos(beta), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}


// An example struct for Game Objects.
// You are encouraged to customize this as you see fit.
struct GameObject {
	// Struct's constructor deals with the texture.
	// Also sets default position, theta, scale, and transformationMatrix
	//GameObject(std::string texturePath, GLenum textureInterpolation) :
	GameObject(std::shared_ptr<GameTexture> g, float x, float y) :
		cgeom(shipGeom(x, y)),
		ggeom(),
		texture(g),
		//texture(texturePath, textureInterpolation),
		position(0.0f, 0.0f, 0.0f),
		theta(-90),
		scale(1),
		transformationMatrix(1.0f) // This constructor sets it as the identity matrix
	{}

	void doTransformation() {
		for (int i = 0; i < cgeom.verts.size(); i++) {
			cgeom.verts[i] = transformationMatrix * glm::vec4(cgeom.verts[i], 1.0f);
		}
		transformationMatrix = identityMatrix();
	}
	void transformPosition(float x, float y) {
		position = transformationMatrix * glm::vec4(position, 1.0f);
		
		cgeom.verts[0] = glm::vec3(position.x - x, position.y + y, 0.f);
		cgeom.verts[1] = glm::vec3(position.x - x, position.y - y, 0.f);
		cgeom.verts[2] = glm::vec3(position.x + x, position.y - y, 0.f);
		cgeom.verts[3] = glm::vec3(position.x - x, position.y + y, 0.f);
		cgeom.verts[4] = glm::vec3(position.x + x, position.y - y, 0.f);
		cgeom.verts[5] = glm::vec3(position.x + x, position.y + y, 0.f);

		transformationMatrix = identityMatrix();
	}

	void resetShip() {
		cgeom = shipGeom(0.18f, 0.14f);
		position.x = 0.f;
		position.y = 0.f;
		theta = -90;
	}

	std::shared_ptr<GameTexture> texture;

	CPU_Geometry cgeom;
	GPU_Geometry ggeom;
	//Texture texture;

	glm::vec3 position;
	float theta; // Object's rotation
	// Alternatively, you could represent rotation via a normalized heading vec:
	// glm::vec3 heading;
	float scale; // Or, alternatively, a glm::vec2 scale;
	glm::mat4 transformationMatrix;
};

glm::vec2 getXY(float theta) {
	glm::vec2 Vec(0.f, 0.0f);
	float ang;
	if (theta < 0) {
		ang = -theta;
	}
	else {
		ang = 360 - theta;
	}
	ang = glm::radians(ang);
	Vec.y = 0.01f * sin(ang);
	Vec.x = 0.01f * cos(ang);

	float distance = sqrtf(pow((Vec.y), 2) + pow((Vec.x), 2));
	Vec.y = Vec.y / distance;
	Vec.x = Vec.x / distance;

	Vec.y = 0.01f * Vec.y;
	Vec.x = 0.01f * Vec.x;

	return Vec;
}
// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader, int screenWidth, int screenHeight, GameObject& sh) :
		reset(false), changes(0),
		screenDim(screenWidth, screenHeight), shader(shader),
		player(sh)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			shader.recompile();
		}
		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
			reset = true;
		}
		if (key == GLFW_KEY_UP) {
			glm::vec2 move = getXY(player.theta);
			player.transformationMatrix = translationMatrix(move.x, move.y) * player.transformationMatrix;
			player.position.x = player.position.x + move.x;
			player.position.y = player.position.y + move.y;
			changes = changes + 2;
		}
		if (key == GLFW_KEY_DOWN) {
			glm::vec2 move = getXY(player.theta);
			player.transformationMatrix = translationMatrix(-move.x, -move.y) * player.transformationMatrix;
			player.position.x = player.position.x - move.x;
			player.position.y = player.position.y - move.y;
			changes = changes + 3;
		}
	}

	glm::vec2 mouseGL() {
		glm::vec2 startingVec(xScreenPos, yScreenPos);
		glm::vec2 shiftedVec = startingVec + glm::vec2(0.5f, 0.5f);
		glm::vec2 scaledToZeroOne = shiftedVec / glm::vec2(screenDim);

		glm::vec2 flippedY = glm::vec2(scaledToZeroOne.x, 1.0f - scaledToZeroOne.y);

		glm::vec2 final = flippedY * 2.0f - glm::vec2(1.0f, 1.0f);

		return final;
	}

	float findAngle(glm::vec2 mousePosition) {
		float x = mousePosition.x - player.position.x;
		float y = mousePosition.y - player.position.y;
		float a = atan(y / x);
		a = a * 180 / M_PI;
		if (x < 0) {
			a = a + 180;
		}
		else if (x > 0 && y < 0) {
			a = a + 360;
		}
		a = 360 - a;
		return a;
	}

	virtual void cursorPosCallback(double xpos, double ypos) {
		xScreenPos = xpos;
		yScreenPos = ypos;
		changes++;
		float angle = findAngle(mouseGL());
		player.transformationMatrix = translationMatrix(player.position.x, player.position.y) * rotationMatrix((player.theta - angle)) * translationMatrix(-player.position.x, -player.position.y) * player.transformationMatrix;
		player.theta = angle;
		//player.doTransformation();
	}

	/*State getState() {
		return state;
	}*/
	bool reset;
	int changes;

private:
	bool leftMousePressed;
	bool rightMousePressed;

	glm::ivec2 screenDim;

	double xScreenPos;
	double yScreenPos;


	ShaderProgram& shader;
	//State state;
	GameObject& player;
	//CPU_Geometry& gg;
};

// END EXAMPLES
//float t = 0.0f;

void setScene(std::vector<std::shared_ptr<GameObject>>& diamonds, std::vector<std::shared_ptr<GameObject>>& fires) {
	std::shared_ptr<GameTexture> shipGeometry = std::make_shared<GameTexture>(
		"textures/ship.png",
		GL_NEAREST
		);

	std::shared_ptr<GameTexture> diamondGeometry = std::make_shared<GameTexture>(
		"textures/diamond.png",
		GL_NEAREST
		);

	std::shared_ptr<GameTexture> fireGeometry = std::make_shared<GameTexture>(
		"textures/fire.png",
		GL_NEAREST
		);

	std::shared_ptr<GameObject> diamond1 = std::make_shared<GameObject>(diamondGeometry, 0.14f, 0.14f);
	std::shared_ptr<GameObject> diamond2 = std::make_shared<GameObject>(diamondGeometry, 0.14f, 0.14f);
	std::shared_ptr<GameObject> diamond3 = std::make_shared<GameObject>(diamondGeometry, 0.14f, 0.14f);
	std::shared_ptr<GameObject> diamond4 = std::make_shared<GameObject>(diamondGeometry, 0.14f, 0.14f);
	std::shared_ptr<GameObject> diamond5 = std::make_shared<GameObject>(diamondGeometry, 0.14f, 0.14f);
	std::shared_ptr<GameObject> diamond6 = std::make_shared<GameObject>(diamondGeometry, 0.14f, 0.14f);
	std::shared_ptr<GameObject> diamond7 = std::make_shared<GameObject>(diamondGeometry, 0.14f, 0.14f);
	std::shared_ptr<GameObject> diamond8 = std::make_shared<GameObject>(diamondGeometry, 0.14f, 0.14f);

	diamonds.push_back(diamond1);
	diamonds.push_back(diamond2);
	diamonds.push_back(diamond3);
	diamonds.push_back(diamond4);
	diamonds.push_back(diamond5);
	diamonds.push_back(diamond6);
	diamonds.push_back(diamond7);
	diamonds.push_back(diamond8);

	std::shared_ptr<GameObject> fire1 = std::make_shared<GameObject>(fireGeometry, 0.10f, 0.14f);
	std::shared_ptr<GameObject> fire2 = std::make_shared<GameObject>(fireGeometry, 0.10f, 0.14f);
	std::shared_ptr<GameObject> fire3 = std::make_shared<GameObject>(fireGeometry, 0.10f, 0.14f);
	std::shared_ptr<GameObject> fire4 = std::make_shared<GameObject>(fireGeometry, 0.10f, 0.14f);
	std::shared_ptr<GameObject> fire5 = std::make_shared<GameObject>(fireGeometry, 0.10f, 0.14f);
	std::shared_ptr<GameObject> fire6 = std::make_shared<GameObject>(fireGeometry, 0.10f, 0.14f);
	std::shared_ptr<GameObject> fire7 = std::make_shared<GameObject>(fireGeometry, 0.10f, 0.14f);
	std::shared_ptr<GameObject> fire8 = std::make_shared<GameObject>(fireGeometry, 0.10f, 0.14f);

	fires.push_back(fire1);
	fires.push_back(fire2);
	fires.push_back(fire3);
	fires.push_back(fire4);
	fires.push_back(fire5);
	fires.push_back(fire6);
	fires.push_back(fire7);
	fires.push_back(fire8);

	float m[8][2] = {
			{0.5, 0.5} ,
			{-0.5, -0.5} ,
			{-0.66, 0.43} ,
			{0.05, -0.73} ,
			{-0.28, 0.31} ,
			{0.69, 0.42} ,
			{-0.71, -0.2} ,
			{0.22, 0.5} ,
	};

	for (int i = 0; i < diamonds.size(); i++) {
		diamonds[i]->ggeom.setVerts(diamonds[i]->cgeom.verts);
		diamonds[i]->ggeom.setTexCoords(diamonds[i]->cgeom.texCoords);
		diamonds[i]->position.x = 0.f;
		diamonds[i]->position.y = 0.f;

		fires[i]->ggeom.setVerts(fires[i]->cgeom.verts);
		fires[i]->ggeom.setTexCoords(fires[i]->cgeom.texCoords);
		fires[i]->position.x = 0.f;
		fires[i]->position.y = 0.f;

		diamonds[i]->transformationMatrix = translationMatrix(m[i][0], m[i][1]) * diamonds[i]->transformationMatrix;
		diamonds[i]->position.x = diamonds[i]->position.x + m[i][0];
		diamonds[i]->position.y = diamonds[i]->position.y + m[i][1];
		diamonds[i]->doTransformation();

		fires[i]->transformationMatrix = translationMatrix(m[i][0] - 0.20f, m[i][1]) * fires[i]->transformationMatrix;
		fires[i]->position.x = fires[i]->position.x + m[i][0] - 0.20f;
		fires[i]->position.y = fires[i]->position.y + m[i][1];
		fires[i]->doTransformation();
	}
}

void addChild(std::vector<std::shared_ptr<GameObject>>& childDiamond, std::vector<std::shared_ptr<GameObject>>& childFire, GameObject& sh) {
	std::shared_ptr<GameTexture> diamondGeometry = std::make_shared<GameTexture>(
		"textures/diamond.png",
		GL_NEAREST
		);

	std::shared_ptr<GameTexture> fireGeometry = std::make_shared<GameTexture>(
		"textures/fire.png",
		GL_NEAREST
		);

	std::shared_ptr<GameObject> childDiamond1 = std::make_shared<GameObject>(diamondGeometry, 0.07f, 0.07f);


	std::shared_ptr<GameObject> childFire1 = std::make_shared<GameObject>(fireGeometry, 0.05f, 0.07f);

	glm::vec2 move = getXY(sh.theta);
	int s = childDiamond.size() + 1;
	float x;
	float y;
	if (s == 1) {
		x = sh.position.x - 0.3f * 100.f * move.x;
		y = sh.position.y - 0.3f * 100.f * move.y;
	}
	else {
		x = childDiamond[s - 2]->position.x - 0.3f * 100.f * move.x;
		y = childDiamond[s - 2]->position.y - 0.3f * 100.f * move.y;
	}

	childDiamond1->transformationMatrix = translationMatrix(x, y) * childDiamond1->transformationMatrix;
	childDiamond1->position.x = childDiamond1->position.x + x;
	childDiamond1->position.y = childDiamond1->position.y + y;
	childDiamond1->doTransformation();
	childDiamond1->transformationMatrix = translationMatrix(childDiamond1->position.x, childDiamond1->position.y) * rotationMatrix((childDiamond1->theta - sh.theta + 90)) * translationMatrix(-childDiamond1->position.x, -childDiamond1->position.y) * childDiamond1->transformationMatrix;
	childDiamond1->theta = sh.theta;
	childDiamond1->doTransformation();

	childDiamond1->ggeom.setVerts(childDiamond1->cgeom.verts);
	childDiamond1->ggeom.setTexCoords(childDiamond1->cgeom.texCoords);

	childDiamond.push_back(childDiamond1);

	childFire1->transformationMatrix = translationMatrix(childDiamond1->position.x - 0.1f * 100.f * move.x, childDiamond1->position.x - 0.1f * 100.f * move.x) * childFire1->transformationMatrix;
	childFire1->position.x = childDiamond1->position.x - 0.1f * 100.f * move.x;
	childFire1->position.y = childDiamond1->position.x - 0.1f * 100.f * move.y;
	childFire1->doTransformation();

	childFire1->transformationMatrix = translationMatrix(childFire1->position.x, childFire1->position.y) * rotationMatrix((childDiamond1->theta - sh.theta + 90)) * translationMatrix(-childFire1->position.x, -childFire1->position.y) * childFire1->transformationMatrix;
	childFire1->theta = sh.theta;
	childFire1->doTransformation();

	childFire1->ggeom.setVerts(childFire1->cgeom.verts);
	childFire1->ggeom.setTexCoords(childFire1->cgeom.texCoords);

	childFire.push_back(childFire1);
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired


	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// GL_NEAREST looks a bit better for low-res pixel art than GL_LINEAR.
	// But for most other cases, you'd want GL_LINEAR interpolation.
	//GameObject ship("textures/ship.png", GL_NEAREST);

	std::shared_ptr<GameTexture> shipGeometry = std::make_shared<GameTexture>(
		"textures/ship.png",
		GL_NEAREST
		);

	GameObject ship(shipGeometry, 0.18f, 0.12f);

	ship.ggeom.setVerts(ship.cgeom.verts);
	ship.ggeom.setTexCoords(ship.cgeom.texCoords);

	std::vector<std::shared_ptr<GameObject>> diamonds;
	std::vector<std::shared_ptr<GameObject>> fires;
	std::vector<std::shared_ptr<GameObject>> childDiamonds;
	std::vector<std::shared_ptr<GameObject>> childFires;
	setScene(diamonds, fires);

	// CALLBACKS
	//
	// For mouse callbacks
	int change = 0;
	int score = 0;
	bool restart = false;

	std::shared_ptr<MyCallbacks> callbacks = std::make_shared<MyCallbacks>(shader, 800, 800, ship);
	window.setCallbacks(callbacks);

	//window.setCallbacks(std::make_shared<MyCallbacks>(shader, 800, 800, ship.cgeom, ship.ggeom)); // can also update callbacks to new ones

	//std::shared_ptr<MyCallbacks> inputManager = std::make_shared<MyCallbacks>(shader, 800, 800);

	//window.setCallbacks(inputManager);
	float size = 0.14f;
	float t = 0;

	//ship.transformationMatrix = scalingMatrix(1.2f) * ship.transformationMatrix;

	// RENDER LOOP
	while (!window.shouldClose()) {

		/*inputManager->refreshStatuses();

		if (inputManager->mouseButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {
			glm::vec2 testVec = window.screenCursorPos();

			glm::vec2 mousePos = inputManager->mouseGL();

			cpuGeom.verts.push_back(glm::vec3(mousePos, 0.0f));
			cpuGeom.cols.push_back(glm::vec3(1.0f));
			gpuGeom.setVerts(cpuGeom.verts);
			gpuGeom.setCols(cpuGeom.cols);
		}*/

		glfwPollEvents();



		shader.use();

		t += 1.0f / 480.0f;

		GLint myLoc = glGetUniformLocation(shader.getProgram(), "time");
		glUniform1f(myLoc, t);   //puts t into "time" on test.vert

		//const GLfloat f(1.0f);

		////std::cout << "Our result: " << f << std::endl;

	

		if (restart || callbacks->reset) {
			int s = diamonds.size();
			for (int j = 0; j < s; j++) {
				diamonds.erase(diamonds.begin());
				fires.erase(fires.begin());
			}
			s = childDiamonds.size();
			for (int j = 0; j < s; j++) {
				childFires.erase(childFires.begin());
				childDiamonds.erase(childDiamonds.begin());
			}
			setScene(diamonds, fires);
			ship.resetShip();
			score = 0;
			size = 0.14f;
			callbacks->reset = false;
			restart = false;
		}

		ship.ggeom.setVerts(ship.cgeom.verts);
		ship.ggeom.setTexCoords(ship.cgeom.texCoords);

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (int i = 0; i < diamonds.size(); i++) {
			diamonds[i]->ggeom.setVerts(diamonds[i]->cgeom.verts);
			diamonds[i]->ggeom.setTexCoords(diamonds[i]->cgeom.texCoords);

			diamonds[i]->ggeom.bind();
			diamonds[i]->texture->textures.bind();
			glDrawArrays(GL_TRIANGLES, 0, 6);
			diamonds[i]->texture->textures.unbind();

			fires[i]->ggeom.setVerts(fires[i]->cgeom.verts);
			fires[i]->ggeom.setTexCoords(fires[i]->cgeom.texCoords);

			fires[i]->ggeom.bind();
			fires[i]->texture->textures.bind();
			glDrawArrays(GL_TRIANGLES, 0, 6);
			fires[i]->texture->textures.unbind();

			fires[i]->transformationMatrix = translationMatrix(diamonds[i]->position.x, diamonds[i]->position.y) * rotationMatrix(0.03f) * translationMatrix(-diamonds[i]->position.x, -diamonds[i]->position.y) * fires[i]->transformationMatrix;
			fires[i]->transformPosition(0.05f, 0.07f);

			float distance = sqrtf(pow((diamonds[i]->position.x - ship.position.x), 2) + pow((diamonds[i]->position.y - ship.position.y), 2));
			float distance2 = sqrtf(pow((fires[i]->position.x - ship.position.x), 2) + pow((fires[i]->position.y - ship.position.y), 2));
			//std::cout << "Our result: " << size << std::endl;
			if (distance < size) {
				//std::cout << "Our result: " << distance << std::endl;
				ship.transformationMatrix = translationMatrix(ship.position.x, ship.position.y) * scalingMatrix(1.15f) * translationMatrix(-ship.position.x, -ship.position.y) * ship.transformationMatrix;
				ship.doTransformation();
				size = size * 1.08f;
				diamonds.erase(diamonds.begin() + i);
				fires.erase(fires.begin() + i);
				addChild(childDiamonds, childFires, ship);
				score++;
			}
			//std::cout << "Our result: " << size << std::endl;
			if (distance2 < size) {
				restart = true;
			}

		}

		//std::cout << "Our result: " << childDiamonds.size() << std::endl;
		for (int i = 0; i < childDiamonds.size(); i++) {

			childDiamonds[i]->ggeom.setVerts(childDiamonds[i]->cgeom.verts);
			childDiamonds[i]->ggeom.setTexCoords(childDiamonds[i]->cgeom.texCoords);

			childDiamonds[i]->ggeom.bind();
			childDiamonds[i]->texture->textures.bind();
			glDrawArrays(GL_TRIANGLES, 0, 6);
			childDiamonds[i]->texture->textures.unbind();

			float distance = sqrtf(pow((childDiamonds[i]->position.x - ship.position.x), 2) + pow((childDiamonds[i]->position.y - ship.position.y), 2));

			if (i == 0) {
				while (distance > 0.3f) {
					float deltaX = 0.001f;
					float deltaY = 0.001f;
					if (childDiamonds[i]->position.x > ship.position.x) {
						deltaX = -1.0f * deltaX;
					}
					if (childDiamonds[i]->position.y > ship.position.y) {
						deltaY = -1.0f * deltaY;
					}
					for (int j = 0; j < childDiamonds.size(); j++) {
						childDiamonds[j]->transformationMatrix = translationMatrix(deltaX, deltaY) * childDiamonds[j]->transformationMatrix;
						childDiamonds[j]->position.x = childDiamonds[j]->position.x + deltaX;
						childDiamonds[j]->position.y = childDiamonds[j]->position.y + deltaY;

						childDiamonds[j]->doTransformation();
					}

					distance = sqrtf(pow((childDiamonds[i]->position.x - ship.position.x), 2) + pow((childDiamonds[i]->position.y - ship.position.y), 2));
				}

				while (distance < 0.28f) {
					float deltaX = -0.001f;
					float deltaY = -0.001f;
					if (childDiamonds[i]->position.x > ship.position.x) {
						deltaX = -1.0f * deltaX;
					}
					if (childDiamonds[i]->position.y > ship.position.y) {
						deltaY = -1.0f * deltaY;
					}
					for (int j = 0; j < childDiamonds.size(); j++) {
						childDiamonds[j]->transformationMatrix = translationMatrix(deltaX, deltaY) * childDiamonds[j]->transformationMatrix;
						childDiamonds[j]->position.x = childDiamonds[j]->position.x + deltaX;
						childDiamonds[j]->position.y = childDiamonds[j]->position.y + deltaY;

						childDiamonds[j]->doTransformation();
					}

					distance = sqrtf(pow((childDiamonds[i]->position.x - ship.position.x), 2) + pow((childDiamonds[i]->position.y - ship.position.y), 2));
				}
			}


			distance = sqrtf(pow((childDiamonds[i]->position.x - childFires[i]->position.x), 2) + pow((childDiamonds[i]->position.y - childFires[i]->position.y), 2));

			while (distance > 0.1f) {
				float deltaX = 0.001f;
				float deltaY = 0.001f;
				if (childDiamonds[i]->position.x < childFires[i]->position.x) {
					deltaX = -1.0f * deltaX;
				}
				if (childDiamonds[i]->position.y < childFires[i]->position.y) {
					deltaY = -1.0f * deltaY;
				}
				childFires[i]->transformationMatrix = translationMatrix(deltaX, deltaY) * childFires[i]->transformationMatrix;
				childFires[i]->position.x = childFires[i]->position.x + deltaX;
				childFires[i]->position.y = childFires[i]->position.y + deltaY;

				childFires[i]->doTransformation();

				distance = sqrtf(pow((childDiamonds[i]->position.x - childFires[i]->position.x), 2) + pow((childDiamonds[i]->position.y - childFires[i]->position.y), 2));
			}

			while (distance < 0.098f) {
				float deltaX = -0.001f;
				float deltaY = -0.001f;
				if (childDiamonds[i]->position.x < childFires[i]->position.x) {
					deltaX = -1.0f * deltaX;
				}
				if (childDiamonds[i]->position.y < childFires[i]->position.y) {
					deltaY = -1.0f * deltaY;
				}
				childFires[i]->transformationMatrix = translationMatrix(deltaX, deltaY) * childFires[i]->transformationMatrix;
				childFires[i]->position.x = childFires[i]->position.x + deltaX;
				childFires[i]->position.y = childFires[i]->position.y + deltaY;

				childFires[i]->doTransformation();

				distance = sqrtf(pow((childDiamonds[i]->position.x - childFires[i]->position.x), 2) + pow((childDiamonds[i]->position.y - childFires[i]->position.y), 2));
			}

			childFires[i]->transformationMatrix = translationMatrix(childDiamonds[i]->position.x, childDiamonds[i]->position.y) * rotationMatrix(0.03f) * translationMatrix(-childDiamonds[i]->position.x, -childDiamonds[i]->position.y) * childFires[i]->transformationMatrix;
			childFires[i]->transformPosition(0.025f, 0.035f);
			childFires[i]->ggeom.setVerts(childFires[i]->cgeom.verts);
			childFires[i]->ggeom.setTexCoords(childFires[i]->cgeom.texCoords);

			childFires[i]->ggeom.bind();
			childFires[i]->texture->textures.bind();
			glDrawArrays(GL_TRIANGLES, 0, 6);
			childFires[i]->texture->textures.unbind();

		}
		//glDrawArrays(GL_LINE_STRIP, 0, cpuGeom.verts.size());
		if (!(change == callbacks->changes)) {
			//ship.cgeom.verts.clear();
			glm::vec2 move = getXY(ship.theta);
			if (callbacks->changes - change > 1) {
				if (callbacks->changes - change > 2) {
					move = -1.f * move;
					change++;
				}
				change = change + 2;
				for (int i = 0; i < childDiamonds.size(); i++) {
					childDiamonds[i]->transformationMatrix = translationMatrix(move.x, move.y) * childDiamonds[i]->transformationMatrix;
					childDiamonds[i]->position.x = childDiamonds[i]->position.x + move.x;
					childDiamonds[i]->position.y = childDiamonds[i]->position.y + move.y;
					childDiamonds[i]->doTransformation();

					childFires[i]->transformationMatrix = translationMatrix(move.x, move.y) * childFires[i]->transformationMatrix;
					childFires[i]->position.x = childFires[i]->position.x + move.x;
					childFires[i]->position.y = childFires[i]->position.y + move.y;
					childFires[i]->doTransformation();
				}
			}
			if (!(change == callbacks->changes)) {
				for (int i = 0; i < childDiamonds.size(); i++) {
					childDiamonds[i]->transformationMatrix = translationMatrix(ship.position.x, ship.position.y) * rotationMatrix((childDiamonds[i]->theta - ship.theta)) * translationMatrix(-ship.position.x, -ship.position.y) * childDiamonds[i]->transformationMatrix;
					childDiamonds[i]->position.x = childDiamonds[i]->cgeom.verts[0].x + (childDiamonds[i]->cgeom.verts[2].x - childDiamonds[i]->cgeom.verts[0].x) / 2;
					childDiamonds[i]->position.y = childDiamonds[i]->cgeom.verts[0].y + (childDiamonds[i]->cgeom.verts[2].y - childDiamonds[i]->cgeom.verts[0].y) / 2;
					childDiamonds[i]->doTransformation();
					childDiamonds[i]->theta = ship.theta;
				}
			}
			change = callbacks->changes;

			ship.doTransformation();
			ship.ggeom.setVerts(ship.cgeom.verts);
		}

		myLoc = glGetUniformLocation(shader.getProgram(), "transformationMatrix");
		glUniformMatrix4fv(myLoc, 1, false, glm::value_ptr(ship.transformationMatrix));
		/*glGetActiveUniform(shader.getProgram(), )*/

		ship.ggeom.bind();
		ship.texture->textures.bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		ship.texture->textures.unbind();

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		// Starting the new ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// Putting the text-containing window in the top-left of the screen.
		ImGui::SetNextWindowPos(ImVec2(5, 5));

		// Setting flags
		ImGuiWindowFlags textWindowFlags =
			ImGuiWindowFlags_NoMove |				// text "window" should not move
			ImGuiWindowFlags_NoResize |				// should not resize
			ImGuiWindowFlags_NoCollapse |			// should not collapse
			ImGuiWindowFlags_NoSavedSettings |		// don't want saved settings mucking things up
			ImGuiWindowFlags_AlwaysAutoResize |		// window should auto-resize to fit the text
			ImGuiWindowFlags_NoBackground |			// window should be transparent; only the text should be visible
			ImGuiWindowFlags_NoDecoration |			// no decoration; only the text should be visible
			ImGuiWindowFlags_NoTitleBar;			// no title; only the text should be visible

		// Begin a new window with these flags. (bool *)0 is the "default" value for its argument.
		ImGui::Begin("scoreText", (bool*)0, textWindowFlags);

		// Scale up text a little, and set its value
		ImGui::SetWindowFontScale(1.5f);
		if (diamonds.size() == 0) {
			ImGui::Text("Congratulations! You Won! Press [ENTER] to start a new game");
		}
		else {
			ImGui::Text("Score: %d", score); // Second parameter gets passed into "%d"
		}

		// End the window.
		ImGui::End();

		ImGui::Render();	// Render the ImGui window
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Some middleware thing

		window.swapBuffers();
	}
	// ImGui cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}
