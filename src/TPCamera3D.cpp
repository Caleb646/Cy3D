#include "pch.h"

//#include "../Sprites/BaseSprite.h"
//#include "TPCamera3D.h"
//
//
//TPCamera3D::TPCamera3D(BaseSprite* _player, Vec3f pos, Vec3f pOffset, Vec3f wUp, float yaw0, float pitch0) : BaseCamera(_player, pos, wUp, yaw0, pitch0), offsetFromPlayer(pOffset)
//{
//    updateCameraVectors();
//    updateProjectionMatrix();
//};
//
///*
//*
//*
//* UPDATE CAMERA METHODS
//*
//*
//*/
//
//void TPCamera3D::processKeyBoard(eMoveDirections dir, float deltaTime)
//{
//    float velocity = moveSpeed * deltaTime;
//    switch (dir)
//    {
//        case eMoveDirections::FORWARD: {
//            cPosition += cLookDir * velocity;
//            player->setPosition(cPosition + offsetFromPlayer);
//            break;
//            }
//        case eMoveDirections::BACKWARD: {
//            cPosition -= cLookDir * velocity;
//            player->setPosition(cPosition + offsetFromPlayer);
//            break;
//        }
//        case eMoveDirections::LEFT: {
//            cPosition -= cRight * velocity;
//            player->setPosition(cPosition + offsetFromPlayer);
//            break;
//        }
//        case eMoveDirections::RIGHT: {
//            cPosition += cRight * velocity;
//            player->setPosition(cPosition + offsetFromPlayer);
//            break;
//        }
//    }
//    updateCameraVectors();
//}
//
//void TPCamera3D::processMouseMove(float xoffset, float yoffset, bool constrainPitch)
//{
//    xoffset *= mouseSensitivity;
//    yoffset *= mouseSensitivity;
//
//    yaw += xoffset;
//    pitch += yoffset;
//
//    if (constrainPitch)
//    {
//        if (pitch > 89.0f)
//            pitch = 89.0f;
//        if (pitch < -89.0f)
//            pitch = -89.0f;
//    }
//    updateCameraVectors();
//}
//
///*
//*
//*
//* PRIVATE METHODS
//*
//*
//*/
//
//void TPCamera3D::updateProjectionMatrix()
//{
//    //TODO camera needs a way to access the current screen width and height
//    projectionMatrix.getPerspectived(degreesToRadians(this->zoom), 800.0f / 600.0f, 0.1f, 100.0f);
//}
//
//void TPCamera3D::updateCameraVectors()
//{
//    float rYaw = degreesToRadians(yaw);
//    float rPitch = degreesToRadians(pitch);
//
//    //Mat4f playerRotateX = Mat4f::getRotateX(rYaw);
//    //Mat4f playerRotateY = Mat4f::getRotateY(rPitch);
//    //Vec4f newPlayerPos;
//    //newPlayerPos *= playerRotateY * playerRotateX * Vec4f(cPosition.x(), cPosition.y(), cPosition.z(), 0.0f) + Vec4f(offsetFromPlayer.x(), offsetFromPlayer.y(), offsetFromPlayer.z(), 0.0f);
//    //ContextManager::get().getPlayer()->position = newPlayerPos.xyz();
//
//
//    Vec3f cNewLookDir;
//    cNewLookDir.x() = std::cos(rYaw) * std::cos(rPitch);
//    cNewLookDir.y() = std::sin(rPitch);
//    cNewLookDir.z() = std::sin(rYaw) * std::cos(rPitch);
//
//    cLookDir = cNewLookDir;
//    cRight = Vec3f::cross(cLookDir, worldUp);
//    cUp = Vec3f::cross(cRight, cLookDir);
//
//    cLookDir.normalized();
//    cRight.normalized();
//    cUp.normalized();
//}
//
//
///*
//* 
//* 
//* 
//* 
//* GETTERS AND SETTERS
//* 
//* 
//* 
//* 
//*/
//
//
//Mat4f TPCamera3D::getLookAt()
//{
//    return Mat4f::getLookAt(cPosition, cPosition + cLookDir, cUp);
//}
//
//Mat4f TPCamera3D::getProjectionMatrix()
//{
//    return projectionMatrix;
//}