#include "ShadowZScene.h"

#define EMPTY_VALUE 0xffffffff

using glm::vec3;

#include <iostream>
#include <GLFW\glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <time.h>

ShadowZScene::ShadowZScene(int w, int h, int numObjects) : IScene(w, h), numObjects(numObjects) {}

void ShadowZScene::initScene() {
	compileAndLinkShaders();

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearStencil(0);

	light.lightPosition = vec4(0.0, 0.0, 0.0, 1.0);
	light.lightColor = vec3(1.0);

	glEnable(GL_DEPTH_TEST);

	angle = 0.0f;

	srand(time(NULL));
	for (int i = 0; i < 38; i++) {
		float r = ((float)(std::rand() % 1000)) * 0.001;
		r = r < 0.1 ? 0.2 : r;
		float g = ((float)(std::rand() % 1000)) * 0.001;
		float b = ((float)(std::rand() % 1000)) * 0.001;
		colors.push_back(glm::vec3(r, g, b));
	}

	sphere = new Mesh("../media/sphere.obj");
	object1 = new MeshAdy("../media/torus_knot.obj");
	object2 = new MeshAdy("../media/monkeyhead.obj");

	setupFBO();

	const float sphereVertexPos[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
	};
	GLuint sphereVertexVBO;
	glGenBuffers(1, &sphereVertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVertexVBO);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float)* 3, sphereVertexPos, GL_STATIC_DRAW);

	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, sphereVertexVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	this->animate(true);
}

void ShadowZScene::setupFBO() {
	// The depth buffer
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

	// The ambient buffer
	GLuint ambientBuffer;
	glGenRenderbuffers(1, &ambientBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, ambientBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);

	// The diffuse + specular component
	glActiveTexture(GL_TEXTURE0);
	GLuint diffuseSpecTexture;
	glGenTextures(1, &diffuseSpecTexture);
	glBindTexture(GL_TEXTURE_2D, diffuseSpecTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Create and set up the FBO
	glGenFramebuffers(1, &colorDepthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, colorDepthFBO);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ambientBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, diffuseSpecTexture, 0);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, drawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Framebuffer isn´t complete." << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowZScene::update(float t) {
	if (!animating()) {
		angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.001f / 3 * t;
		light.lightPosition.x = sin(glfwGetTime() * 0.5) * 5.0;
		light.lightPosition.z = cos(glfwGetTime() * 0.5) * 5.0;
	}
}

void ShadowZScene::draw(Camera* cam) {
	view = cam->GetViewMatrix();
	light.lp = glm::vec3(view * light.lightPosition);
	pass1Render(cam);
	pass2Volume(cam);
	pass3Composite();
}

void ShadowZScene::pass1Render(Camera* cam) {
	glDepthMask(GL_TRUE);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	projection = cam->GetProjectionMatrix();

	renderShader.use();
	renderShader.send_uniform("lightPosition", light.lp);
	renderShader.send_uniform("lightColor", light.lightColor);
	renderShader.send_uniform("viewPos", cam->Position);

	glBindFramebuffer(GL_FRAMEBUFFER, colorDepthFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	drawScene(renderShader, false);
}

void ShadowZScene::pass2Volume(Camera* cam) {
	volumeShader.use();
	volumeShader.send_uniform("lightPosition", light.lp);
	projection = cam->GetProjectionInfiniteMatrix();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, colorDepthFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// Copy a pixels from read framebuffer to draw framebuffer
	glBlitFramebuffer(
		0, 0, width, height,
		0, 0, width, height,
		GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);

	glStencilFunc(GL_ALWAYS, 0, EMPTY_VALUE);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);

	drawScene(volumeShader, true);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void ShadowZScene::pass3Composite() {
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	// Only render, where the stencil buffer is not 0
	glStencilFunc(GL_EQUAL, 0, EMPTY_VALUE);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	compositeShader.use();

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void ShadowZScene::drawScene(SimpleGLShader &shader, bool objectCastShadow) {
	vec3 color;

	if(!objectCastShadow ) {
		color = vec3(1.0f);
		shader.send_uniform("Color", color);
	}
	std::srand(1234);
	for (unsigned int i = 0; i < numObjects; i++) {
		float size = float(std::rand() % 3 + 1);

		glm::vec3 axis(glm::vec3(float(std::rand() % 2), float(std::rand() % 2), float(std::rand() % 2)));
		if (glm::all(glm::equal(axis, glm::vec3(0.0f))))
			axis = glm::vec3(1.0f);

		float trans = float(std::rand() % 7 + 3) * 1.00f + 0.5f;
		glm::vec3 transVec = axis * trans;
		transVec.x *= (std::rand() % 2) ? 1.0f : -1.0f;
		transVec.y *= (std::rand() % 2) ? 1.0f : -1.0f;
		transVec.z *= (std::rand() % 2) ? 1.0f : -1.0f;

		model = glm::rotate(glm::mat4(1.0f), angle*2.0f*size, axis);
		model = glm::translate(model, transVec);
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 0, 1));
		model = glm::rotate(model, angle*2.0f*size, axis);
		model = glm::scale(model, glm::vec3(1.0f / (size*0.7f)));
		setMatrices(shader);
		shader.send_uniform("Color", colors[i % colors.size()]);
		switch (i % 2) {
		case 0:
			object1->render();
			break;
		case 1:
			object2->render();
			break;
		}
	}

	if( !objectCastShadow ) {
		shader.send_uniform("Color", vec3(1.0, 1.0, 0.0));
		model = mat4(1.0f);
		model = glm::scale(model, vec3(10.0f));
		setMatrices(shader);
		sphere->render();

		// Light
		model = glm::translate(glm::mat4(), vec3(light.lightPosition));
		model = glm::scale(model, glm::vec3(0.2));
		shader.send_uniform("Color", light.lightColor);
		setMatrices(shader);
		sphere->render();
	}
}

void ShadowZScene::setMatrices(SimpleGLShader &shader) {
    mat4 mv = view * model;
    shader.send_uniform("modelView", mv);
    shader.send_uniform("projection", projection);
    shader.send_uniform("normal", mat3( vec3(mv[0]), vec3(mv[1]), vec3(mv[2]) ));
}

void ShadowZScene::resize(int w, int h) {
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}

void ShadowZScene::compileAndLinkShaders() {
	renderShader.load("../shaders/renderShader.vert", "../shaders/renderShader.frag");
	renderShader.compile_and_link();
	volumeShader.add_uniform("lightPosition");
	volumeShader.add_uniform("viewPos");
	volumeShader.add_uniform("modelView");
	volumeShader.add_uniform("projection");
	volumeShader.add_uniform("normal");

	volumeShader.load("../shaders/volumeShader.vert", GL_VERTEX_SHADER);
	volumeShader.load("../shaders/volumeShader.geom", GL_GEOMETRY_SHADER);
	volumeShader.compile_and_link();
	volumeShader.add_uniform("lightPosition");
	volumeShader.add_uniform("modelView");
	volumeShader.add_uniform("projection");
	volumeShader.add_uniform("normal");
	
	compositeShader.load("../shaders/compositeShader.vert", "../shaders/compositeShader.frag");
	compositeShader.compile_and_link();
}
