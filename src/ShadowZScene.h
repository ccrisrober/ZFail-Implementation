#ifndef SHADOWZSCENE_H
#define SHADOWZSCENE_H

#include "IScene.h"
#include "SimpleGLShader.h"
#include "objects/RenderObject.h"
#include "objects/Mesh.h"
#include "objects/MeshAdy.h"

#include <GL/glew.h>

#include <glm/glm.hpp>

class ShadowZScene : public IScene {
private:
	SimpleGLShader volumeShader, renderShader, compositeShader;
	GLuint colorDepthFBO, quadVAO;

	RenderObject *sphere;
	RenderObject *object1, *object2;

	struct {
		glm::vec3 lp;
		vec4 lightPosition;
		vec3 lightColor;
	} light;

	float angle;
	std::vector<glm::vec3> colors;

	void setMatrices(SimpleGLShader& shader);
	void compileAndLinkShaders();
    void setupFBO();
	void drawScene(SimpleGLShader& shader, bool objectCastShadow);
	void pass1Render(Camera* cam);
	void pass2Volume(Camera* cam);
	void pass3Composite();
	int numObjects = 5;

public:
	ShadowZScene(int w, int h, int numObjects = 5);

    void initScene();
    void update( float t );
    void draw(Camera* camera);
	void resize(int, int); 
	
	int objectsInScene() { return numObjects; }
	void addNewObject() {
		numObjects++;
	}
	void removeObject() {
		if (numObjects > 1)
			numObjects--;
	}
};

#endif // SHADOWZSCENE_H
