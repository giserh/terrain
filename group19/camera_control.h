
#ifndef __camera_control_h__
#define __camera_control_h__

#include <vector>
#include <GL/glew.h>
#include "opengp.h"

/// Forward declarations (no header includes) (namespace pollution, build time).
class VerticesCameraPath;

class CameraControl {

public:

    /// Camera control interface.
    void init(VerticesCameraPath* verticesCameraPath, unsigned int heightMapTexID);
    void trackball(const mat4& model);
    void updateCameraPosition(mat4& cameraModelview);
    void handleCameraControls(int key, int action);

private:

    /// Camera model view transformation matrix.
    mat4 _cameraModelview;

    /// Camera path (Bézier curve) coordinates.
    std::vector<float> _bezierCurve;

    /// Handle to camera path vertices.
    VerticesCameraPath* _verticesCameraPath;

    /// Heightmap texture (copied back to CPU).
    float* _heightmapCPU;

    /// World exploration mode.
    enum exploration_mode {FPS, FLYING, PATH, TRACKBALL};
    enum exploration_mode _explorationMode;

    /// Helper functions.
    void update_camera_modelview(double posX,double posY,double posZ,double lookX,double lookY,double lookZ);
    void handleKeyboard();
    void rotate2D(double pos1, double pos2, double& look1, double& look2, double angle);
    void rotate3D(double& posX, double& posY, double& posZ, double& lookX, double& lookY, double& lookZ,double& recordRotY,double& recordRotZ);
    void moveAlongAxis(double& posX, double& posY, double& posZ, double& lookX, double& lookY, double& lookZ, double velocity);
    void fpsExplorationForwardBackward(double& posX, double& posY, double& posZ, double& lookX, double& lookY, double& lookZ,double dispX,double dispY);
    void deCasteljauTest3Points();
    void deCasteljauTest4Points();
    void InitdeCasteljau4Points();
    void flyingExploration();
    void fpsExploration();
    void fpsRotateLeftRight(double& posX, double& posY, double& posZ, double& lookX, double& lookY, double& lookZ,double velocity);
    void fpsRotateUpDown(double& posX, double& posY, double& posZ, double& lookX, double& lookY, double& lookZ,double recordRotZ, double velocity);
    void rotateUpDown(double& posX, double& posY, double& posZ, double& lookX, double& lookY, double& lookZ,double& recordRotZ,double velocity);
    void rotateLeftRight(double& posX, double& posY, double& posZ, double& lookX, double& lookY, double& lookZ,double& recordRotY,double velocity);
    void InitdeCasteljauSubdivision();

};

#endif /* __camera_control_h__ */