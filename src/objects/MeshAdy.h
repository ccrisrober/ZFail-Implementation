#ifndef MeshAdy_H
#define MeshAdy_H

#include "RenderObject.h"

#include <vector>
#include <glm/glm.hpp>
using glm::vec3;
using glm::vec2;
using glm::vec4;

#include <string>


class MeshAdy : public RenderObject {
private:
    void trimString( std::string & str );
    void determineAdjacency( std::vector<GLuint> & el );
    void storeVBO( const std::vector<vec3> & points,
                            const std::vector<vec3> & normals,
                            const std::vector<vec2> &texCoords,
                            const std::vector<GLuint> &elements );
    void generateAveragedNormals(
            const std::vector<vec3> & points,
            std::vector<vec3> & normals,
            const std::vector<GLuint> & faces );

public:
    MeshAdy( const char * fileName );

    void loadOBJ( const char * fileName );
};

#endif // VBOMESH_H
