#include "skybox.h"
#include "glad/glad.h"

Skybox::Skybox() : coneMesh(nullptr)
{
	
}

Skybox::~Skybox()
{
}

void Skybox::init()
{
    coneMesh = std::make_unique<Mesh>();

    const int   segments = 64;
    const float radius = 1.0f;
    const float apexY = 1.0f;
    const float baseY = 0.0f;

    std::vector<glm::vec3> verts;
    std::vector<glm::vec3> texcoords; 
    std::vector<unsigned int> inds;

    verts.reserve(1 + segments);
    texcoords.reserve(1 + segments);
    inds.reserve(segments * 3);

    // Apex
    verts.emplace_back(0.0f, apexY, 0.0f);
    texcoords.emplace_back(0.5f, 1.0f, 0.0f); 

    const float twoPi = 6.283185307179586f;

    // Base ring
    for (int i = 0; i < segments; ++i) {
        float a = twoPi * (float)i / (float)segments;
        float x = radius * std::cos(a);
        float z = radius * std::sin(a);

        verts.emplace_back(x, baseY, z);

        float u = (float)i / (float)segments; 
        float v = 0.0f;                       
        texcoords.emplace_back(u, v, 0.0f);
    }

    // Side triangles
    for (int i = 0; i < segments; ++i) {
        unsigned int i0 = 0;
        unsigned int i1 = 1 + ((i + 1) % segments);
        unsigned int i2 = 1 + i;
        inds.push_back(i0);
        inds.push_back(i1);
        inds.push_back(i2);
    }

    coneMesh->setVertices(verts);
    coneMesh->setTexCoords(texcoords);
    coneMesh->setIndices(inds);
    coneMesh->setNormals({});
    coneMesh->ao.clear();

    coneMesh->setupMesh();
}


void Skybox::draw()
{
    if (coneMesh)
    {
        glDepthMask(GL_FALSE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        coneMesh->draw();
        glCullFace(GL_BACK);
        glDisable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
    }
}

