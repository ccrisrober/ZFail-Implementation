#include "MeshAdy.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include <map>

MeshAdy::MeshAdy(const char * fileName) {
    loadOBJ(fileName);
	type = GL_TRIANGLES_ADJACENCY;
	_size = 6 * _size;
}
void MeshAdy::determineAdjacency(std::vector<GLuint> &el) {
    // Elements with adjacency info
    std::vector<GLuint> elAdj;

    // Copy and make room for adjacency info
    for( GLuint i = 0; i < el.size(); i+=3) {
        elAdj.push_back(el[i]);
        elAdj.push_back(-1);
        elAdj.push_back(el[i+1]);
        elAdj.push_back(-1);
        elAdj.push_back(el[i+2]);
        elAdj.push_back(-1);
    }

    // Find matching edges
    for( GLuint i = 0; i < elAdj.size(); i+=6) {
        // A triangle
        int a1 = elAdj[i];
        int b1 = elAdj[i+2];
        int c1 = elAdj[i+4];

        // Scan subsequent triangles
        for(GLuint j = i+6; j < elAdj.size(); j+=6) {
            int a2 = elAdj[j];
            int b2 = elAdj[j+2];
            int c2 = elAdj[j+4];

            // Edge 1 == Edge 1
            if( (a1 == a2 && b1 == b2) || (a1 == b2 && b1 == a2) ) {
                elAdj[i+1] = c2;
                elAdj[j+1] = c1;
            }
            // Edge 1 == Edge 2
            if( (a1 == b2 && b1 == c2) || (a1 == c2 && b1 == b2) ) {
                elAdj[i+1] = a2;
                elAdj[j+3] = c1;
            }
            // Edge 1 == Edge 3
            if ( (a1 == c2 && b1 == a2) || (a1 == a2 && b1 == c2) ) {
                elAdj[i+1] = b2;
                elAdj[j+5] = c1;
            }
            // Edge 2 == Edge 1
            if( (b1 == a2 && c1 == b2) || (b1 == b2 && c1 == a2) ) {
                elAdj[i+3] = c2;
                elAdj[j+1] = a1;
            }
            // Edge 2 == Edge 2
            if( (b1 == b2 && c1 == c2) || (b1 == c2 && c1 == b2) ) {
                elAdj[i+3] = a2;
                elAdj[j+3] = a1;
            }
            // Edge 2 == Edge 3
            if( (b1 == c2 && c1 == a2) || (b1 == a2 && c1 == c2) ) {
                elAdj[i+3] = b2;
                elAdj[j+5] = a1;
            }
            // Edge 3 == Edge 1
            if( (c1 == a2 && a1 == b2) || (c1 == b2 && a1 == a2) ) {
                elAdj[i+5] = c2;
                elAdj[j+1] = b1;
            }
            // Edge 3 == Edge 2
            if( (c1 == b2 && a1 == c2) || (c1 == c2 && a1 == b2) ) {
                elAdj[i+5] = a2;
                elAdj[j+3] = b1;
            }
            // Edge 3 == Edge 3
            if( (c1 == c2 && a1 == a2) || (c1 == a2 && a1 == c2) ) {
                elAdj[i+5] = b2;
                elAdj[j+5] = b1;
            }
        }
    }

    // Look for any outside edges
    for( GLuint i = 0; i < elAdj.size(); i+=6) {
        if( elAdj[i+1] == -1 ) elAdj[i+1] = elAdj[i+4];
        if( elAdj[i+3] == -1 ) elAdj[i+3] = elAdj[i];
        if( elAdj[i+5] == -1 ) elAdj[i+5] = elAdj[i+2];
    }

    // Copy all data back into el
    el = elAdj;
}

void MeshAdy::loadOBJ( const char * fileName ) {

  std::vector <vec3> p;
  std::vector <vec3> n;
  std::vector <vec2> tc;         // Holds tex coords from OBJ file
  std::vector <GLuint> _size, faceTC;

  int nFaces = 0;

  std::ifstream objStream( fileName, std::ios::in );

  if( !objStream ) {
    std::cerr << "Unable to open OBJ file: " << fileName << std::endl;
    exit(1);
  }

  std::cout << "Loading OBJ mesh: " << fileName << std::endl;
  std::string line, token;

  getline( objStream, line );
  while( !objStream.eof() ) {
    trimString(line);
    if( line.length( ) > 0 && line.at(0) != '#' ) {
      std::istringstream lineStream( line );

      lineStream >> token;

      if (token == "v" ) {
        float x, y, z;
        lineStream >> x >> y >> z;
        p.push_back( vec3(x,y,z) );
      } else if (token == "vt" ) {
        // Process texture coordinate
        float s,t;
        lineStream >> s >> t;
        tc.push_back( vec2(s,t) );
      } else if (token == "vn" ) {
        float x, y, z;
        lineStream >> x >> y >> z;
        n.push_back( vec3(x,y,z) );
      } else if (token == "f" ) {
        nFaces++;

        // Process face
        size_t slash1, slash2;
        int faceVerts = 0;
        while( lineStream.good() ) {
          faceVerts++;
          std::string vertString;
          lineStream >> vertString;
          int pIndex = -1, nIndex = -1 , tcIndex = -1;

          slash1 = vertString.find("/");
          if( slash1 == std::string::npos ){
            pIndex = atoi( vertString.c_str() ) - 1 ;
          } else {
            slash2 = vertString.find("/", slash1 + 1 );
            pIndex = atoi( vertString.substr(0,slash1).c_str() ) - 1;
            if( slash2 == std::string::npos || slash2 > slash1 + 1) {
              tcIndex =
                atoi( vertString.substr(slash1 + 1, slash2).c_str() ) - 1;
            }
            if( slash2 != std::string::npos )
              nIndex =
                atoi( vertString.substr(slash2 + 1,std::string::npos).c_str() ) - 1;
          }
          if( pIndex == -1 ) {
            //printf("Missing point index!!!");
          } else {
            _size.push_back(pIndex);
          }
          if( tcIndex != -1 ) faceTC.push_back(tcIndex);

          if ( nIndex != -1 && nIndex != pIndex ) {
            //printf("Normal and point indices are not consistent.\n");
          }
        }
        if( faceVerts != 3 ) {
          //printf("Found non-triangular face.\n");
        }
      }
    }
    getline( objStream, line );
  }

  objStream.close();

  // 2nd pass, re-do the lists to make the indices consistent
  std::vector<vec2> texCoords;
  for( GLuint i = 0; i < p.size(); i++ ) texCoords.push_back(vec2(0.0f));
  std::map<int, int> pToTex;
  for( GLuint i = 0; i < _size.size(); i++ ) {
    int point = _size[i];
    int texCoord = faceTC[i];
    std::map<int, int>::iterator it = pToTex.find(point);
    if( it == pToTex.end() ) {
      pToTex[point] = texCoord;
      texCoords[point] = tc[texCoord];
    } else {
      if( texCoord != it->second ) {
        p.push_back( p[point] );  // Dup the point
        texCoords.push_back( tc[texCoord] );
        _size[i] = GLuint(p.size() - 1);
      }
    }
  }

  //if( n.size() == 0 ) {
    std::cout << "Generating normal vectors" << std::endl;
    generateAveragedNormals(p,n,_size);
  //}

  // Determine the adjacency information
  std::cout << "Determining mesh adjacencies" << std::endl;
  determineAdjacency(_size);

  storeVBO(p, n, texCoords, _size);

  std::cout << "Loaded mesh from: " << fileName << std::endl;
  std::cout << " " << p.size() << " points" << std::endl;
  std::cout << " " << nFaces << " _size" << std::endl;
  std::cout << " " << n.size() << " normals" << std::endl;
  std::cout << " " << texCoords.size() << " texture coordinates." << std::endl;
}

void MeshAdy::generateAveragedNormals(
        const std::vector<vec3> & points,
        std::vector<vec3> & normals,
        const std::vector<GLuint> & _size )
{
    for( GLuint i = 0; i < points.size(); i++ ) {
        normals.push_back(vec3(0.0f));
    }

    for( GLuint i = 0; i < _size.size(); i += 3) {
        const vec3 & p1 = points[_size[i]];
        const vec3 & p2 = points[_size[i+1]];
        const vec3 & p3 = points[_size[i+2]];

        vec3 a = p2 - p1;
        vec3 b = p3 - p1;
        vec3 n = glm::normalize(glm::cross(a,b));

        normals[_size[i]] += n;
        normals[_size[i+1]] += n;
        normals[_size[i+2]] += n;
    }

    for( GLuint i = 0; i < normals.size(); i++ ) {
        normals[i] = glm::normalize(normals[i]);
    }
}
void MeshAdy::storeVBO( const std::vector<vec3> & points,
                        const std::vector<vec3> & normals,
                        const std::vector<vec2> &texCoords,
                        const std::vector<GLuint> &elements )
{
    GLuint nVerts  = GLuint(points.size());
    _size = GLuint(elements.size() / 6);

    float * v = new float[3 * nVerts];
    float * n = new float[3 * nVerts];
    float * tc = NULL;
    float * tang = NULL;

    if(texCoords.size() > 0) {
        tc = new float[ 2 * nVerts];
    }

    unsigned int *el = new unsigned int[elements.size()];
    int idx = 0, tcIdx = 0, tangIdx = 0;
    for( GLuint i = 0; i < nVerts; ++i )
    {
        v[idx] = points[i].x;
        v[idx+1] = points[i].y;
        v[idx+2] = points[i].z;
        n[idx] = normals[i].x;
        n[idx+1] = normals[i].y;
        n[idx+2] = normals[i].z;
        idx += 3;
        if( tc != NULL ) {
            tc[tcIdx] = texCoords[i].x;
            tc[tcIdx+1] = texCoords[i].y;
            tcIdx += 2;
        }
    }
    for( unsigned int i = 0; i < elements.size(); ++i )
    {
        el[i] = elements[i];
    }
    glGenVertexArrays( 1, &_vao );
    glBindVertexArray(_vao);

    int nBuffers = 5;
    GLuint elementBuffer = 4;
    if( tc == NULL ) {
        nBuffers = 3;
        elementBuffer = 2;
    }

    unsigned int handle[5];
    glGenBuffers(nBuffers, handle);

    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, (3 * nVerts) * sizeof(float), v, GL_STATIC_DRAW);
    glVertexAttribPointer( (GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );
    glEnableVertexAttribArray(0);  // Vertex position

    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ARRAY_BUFFER, (3 * nVerts) * sizeof(float), n, GL_STATIC_DRAW);
    glVertexAttribPointer( (GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );
    glEnableVertexAttribArray(1);  // Vertex normal

    if( tc != NULL ) {
        glBindBuffer(GL_ARRAY_BUFFER, handle[2]);
        glBufferData(GL_ARRAY_BUFFER, (2 * nVerts) * sizeof(float), tc, GL_STATIC_DRAW);
        glVertexAttribPointer( (GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );
        glEnableVertexAttribArray(2);  // Texture coords

        glBindBuffer(GL_ARRAY_BUFFER, handle[3]);
        glBufferData(GL_ARRAY_BUFFER, (4 * nVerts) * sizeof(float), tang, GL_STATIC_DRAW);
        glVertexAttribPointer( (GLuint)3, 4, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );
        glEnableVertexAttribArray(3);  // Tangent std::vector
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[elementBuffer]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * _size * sizeof(unsigned int), el, GL_STATIC_DRAW);

    glBindVertexArray(0);

    delete [] v;
    delete [] n;
    if( tc != NULL ) {
        delete [] tc;
        delete [] tang;
    }
    delete [] el;
    printf("End storeVBO\n");
}

void MeshAdy::trimString( std::string & str ) {
    const char * whiteSpace = " \t\n\r";
    size_t location;
    location = str.find_first_not_of(whiteSpace);
    str.erase(0,location);
    location = str.find_last_not_of(whiteSpace);
    str.erase(location + 1);
}
