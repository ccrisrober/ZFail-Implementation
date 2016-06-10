#pragma once
#include <GL/glew.h>
#include <iostream>
class RenderObject {
public:
	unsigned int _vao;
	unsigned int _size;
	int type = GL_TRIANGLES;

	virtual void render() {
		glBindVertexArray(_vao);
		glDrawElements(type, _size, GL_UNSIGNED_INT, 0);
	}
	GLuint texture = -1;
};

