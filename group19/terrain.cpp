
#include "terrain.h"
#include "vertices.h"

#include <iostream>

#include <GL/glew.h>
#include <GL/glfw.h>
#include "opengp.h"

#include "terrain_vshader.h"
#include "terrain_fshader.h"


Terrain::Terrain(unsigned int width, unsigned int height) :
    RenderingContext(width, height) {
}


void Terrain::init(Vertices* vertices, GLuint heightMapTexID, GLuint shadowMapTexID) {

    /// Common initialization.
    RenderingContext::init(vertices, terrain_vshader, terrain_fshader, "vertexPosition2DModel");

    /// Bind the heightmap and shadowmap to textures 0 and 1.
    set_texture(0, heightMapTexID, "heightMapTex");
    set_texture(1, shadowMapTexID, "shadowMapTex");

    /// Load material textures and bind them to textures 2 - 7.
    set_texture(2, -1, "sandTex");
    load_texture("../../textures/sand.tga");
    set_texture(3, -1, "iceMoutainTex");
    load_texture("../../textures/dordona_range.tga");
    set_texture(4, -1, "treeTex");
    load_texture("../../textures/forest.tga");
    set_texture(5, -1, "stoneTex");
    load_texture("../../textures/stone_2.tga");
    set_texture(6, -1, "waterTex");
    load_texture("../../textures/water_2.tga");
    set_texture(7, -1, "snowTex");
    load_texture("../../textures/snow.tga");


    /// Load the normal map for water lightning to texture 8.
    set_texture(8, -1, "waterNormalMap");
    load_texture("../../textures/water_normal_map_2.tga");

	set_texture(9,-1, "riverSurfaceMap");
	load_texture("../../textures/river.tga");

    /// Define light properties and pass them to the shaders.
    vec3 Ia(1.0f, 1.0f, 1.0f);
    vec3 Id(1.0f, 1.0f, 1.0f);
    vec3 Is(1.0f, 1.0f, 1.0f);
    GLuint _IaID = glGetUniformLocation(_programID, "Ia");
    GLuint _IdID = glGetUniformLocation(_programID, "Id");
    GLuint _IsID = glGetUniformLocation(_programID, "Is");
    glUniform3fv(_IaID, 1, Ia.data());
    glUniform3fv(_IdID, 1, Id.data());
    glUniform3fv(_IsID, 1, Is.data());

    /// Set uniform IDs.
    _modelviewID = glGetUniformLocation(_programID, "modelview");
    _projectionID = glGetUniformLocation(_programID, "projection");
    _lightOffsetMVPID = glGetUniformLocation(_programID, "lightOffsetMVP");
    _lightPositionModelID = glGetUniformLocation(_programID, "lightPositionModel");
    _timeID = glGetUniformLocation(_programID, "time");

}


void Terrain::draw(mat4& projection, mat4& modelview, mat4& lightMVP, vec3& lightPositionModel) const {

    /// Common drawing.
    RenderingContext::draw();

    /// Update the content of the uniforms.
    glUniformMatrix4fv(_modelviewID, 1, GL_FALSE, modelview.data());
    glUniformMatrix4fv(_projectionID, 1, GL_FALSE, projection.data());
    glUniform3fv(_lightPositionModelID, 1, lightPositionModel.data());

    /// Time value which animates water.
    // TODO: implement rollover ?
    static float time = 0;
    glUniform1f(_timeID, int(time++)%5000);

    /// Map from light-coordinates in (-1,1)x(-1,1) to texture
    /// coordinates in (0,1)x(0,1).
    mat4 offsetMatrix;
    offsetMatrix <<
            0.5f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f;
    mat4 lightOffsetMVP = offsetMatrix * lightMVP;
    glUniformMatrix4fv(_lightOffsetMVPID, 1, GL_FALSE, lightOffsetMVP.data());

    /// Clear the screen framebuffer.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /// Render the terrain from camera point of view to default framebuffer.
    _vertices->draw(_vertexAttribID);

}


GLuint Terrain::load_texture(const char * imagepath) const {

    // Read the file, call glTexImage2D with the right parameters
    if (glfwLoadTexture2D(imagepath, 0)) {
        // Nice trilinear filtering.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Cannot load texture file : " << imagepath << std::endl;
        exit(EXIT_FAILURE);
    }

}
