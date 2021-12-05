#pragma once

//#include "BaseCamera.h"
//#include <_vmath/_vmath.h>
//
//
//class TPCamera3D : public BaseCamera
//{
//
//private:
//    Mat4f projectionMatrix;
//    Vec3f offsetFromPlayer;
//
//protected:
//    void updateProjectionMatrix() override;
//    void updateCameraVectors() override;
//
//public:
//    TPCamera3D(BaseSprite* _player, Vec3f pos = Vec3f(0.0f, 0.0f, 0.0f), Vec3f pOffset = Vec3f(0.0f, 0.0f, -1.0f), Vec3f wUp = Vec3f(0.0f, 1.0f, 0.0f), float yaw0 = YAW, float pitch0 = PITCH);
//    void processKeyBoard(eMoveDirections dir, float deltaTime) override;
//    void processMouseMove(float xoffset, float yoffset, bool constrainPitch = true) override;
//    Mat4f getLookAt() override;
//    Mat4f getProjectionMatrix() override;
//
//};