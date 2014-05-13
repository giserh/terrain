
#include "shadowmap.h"
#include "vertices.h"


#include <iostream>

#include <GL/glew.h>
#include <GL/glfw.h>
#include "opengp.h"

#include "shadowmap_vshader.h"
#include "shadowmap_fshader.h"

// FIXME : vertices as an object ?
extern unsigned int nIndices;


Shadowmap::Shadowmap(unsigned int width, unsigned int height) :
    RenderingContext(width, height) {
}


GLuint Shadowmap::init(Vertices* vertices, GLuint heightMapTexID) {

    /// Common initialization : vertex array and shader programs.
    RenderingContext::init(vertices, shadowmap_vshader, shadowmap_fshader, -1);

    /// Bind the heightmap to texture 0.
    set_texture(0, heightMapTexID, "heightMapTex");

    /// Create and bind to texture 1 the texture which will contain the
    /// color / depth output (the actual shadow map) of our shader.
    // Depth texture. Slower than a depth buffer, but we can sample it later in the shader.
    // Do not need to be binded in draw().
    GLuint shadowMapTexID = set_texture(1);

    // Depth format is unsigned int. Set it to 16 bits.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, _width, _height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

    /// Attach the created texture to the depth attachment point.
    /// The texture becomes the fragment shader first output buffer.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _textureIDs[1], 0);

    /// There is only depth (no color) output in the bound framebuffer.
    glDrawBuffer(GL_NONE);

    /// Check that our framebuffer is complete.
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Shadowmap framebuffer not complete." << std::endl;
        exit(EXIT_FAILURE);
    }

    /// Set uniform and attribute IDs.
    _lightMatrixID = glGetUniformLocation(_programID, "lightMVP");

    /// Set vertex attribute array IDs.
    _vertexAttribID = glGetAttribLocation(_programID, "vertexPosition2DModel");

    /// Return the shadowmap texture ID (for the terrain).
    return shadowMapTexID;

}


void Shadowmap::draw(mat4& /*projection*/, mat4& /*modelview*/, mat4& lightMVP) const {

    // Common drawing.
    RenderingContext::draw();

    /// Update the content of the uniforms.
    glUniformMatrix4fv(_lightMatrixID, 1, GL_FALSE, lightMVP.data());

    /// Clear the framebuffer object.
    glClear(GL_DEPTH_BUFFER_BIT);

    /// Render the terrain from light source point of view to FBO.
    _vertices->draw(_vertexAttribID);

}


void Shadowmap::clean() {
    RenderingContext::clean();
}
