#include "Camera.h"



namespace cy3d
{
    Camera::Camera(m3d::Vec3f pos, m3d::Vec3f wUp, float yaw0, float pitch0) : pos(pos), worldUp(wUp), yaw(yaw0), pitch(pitch0) {}

    Camera::~Camera() {}

    void Camera::print()
    {
        std::cout << "Position" << std::endl;
        pos.print();
        std::cout << std::endl;

        std::cout << "Look" << std::endl;
        lookDir.print();
        std::cout << std::endl;

        std::cout << "Right" << std::endl;
        right.print();
        std::cout << std::endl;
    }


    /**
    * 
    * 
    * Public Static Methods
    * 
    * 
    */
    m3d::Mat4f  Camera::createLookAtMatrix(m3d::Vec3f pos, m3d::Vec3f lookDir, m3d::Vec3f cUp)
    {
        return m3d::Mat4f::getLookAt(pos, pos + lookDir, cUp);
    }

    m3d::Mat4f  Camera::createLookAtMatrix(Camera& camera)
    {
        return m3d::Mat4f::getLookAt(camera.pos, camera.pos + camera.lookDir, camera.cUp);
    }

    m3d::Mat4f  Camera::createPerspectiveMatrix(float fov, float aspectRatio, float far, float near)
    {
        return m3d::Mat4f::getPerspective(m3d::degreesToRadians(fov), aspectRatio, far, near);
    }

    Camera* Camera::create3D(Camera* camera, float fov, float aspectRatio, float far, float near)
    {
        camera->projectionMatrix = createPerspectiveMatrix(fov, aspectRatio, far, near);

        //TODO MAKE SURE that the ptr to is not deleted when out of scope. Lambda needs to have the 
        //the make camera ptr. Maybe make it a shared pointer.
        camera->fnKeyBoardInputCallback = [cam = camera](KeyBoardInputEvent& e, double deltaTime)
        {
            float velocity = cam->moveSpeed * deltaTime;
            if (e.key == GLFW_KEY_W && e.action == GLFW_PRESS)
            {
                cam->pos += cam->lookDir * velocity;
            }
            else if (e.key == GLFW_KEY_S && e.action == GLFW_PRESS)
            {
                cam->pos -= cam->lookDir * velocity;
            }
            else if (e.key == GLFW_KEY_A && e.action == GLFW_PRESS)
            {
                cam->pos += cam->right * velocity;
            }
            else if (e.key == GLFW_KEY_D && e.action == GLFW_PRESS)
            {
                cam->pos -= cam->right * velocity;
            }
        };
        //TODO need to create an then assign to the window as a listener
        camera->fnMouseInputCallback = [cam = camera](MouseInputEvent& e, double deltaTime)
        {
            //xoffset *= cam->mouseSensitivity;
            //yoffset *= cam->mouseSensitivity;
            //
            //yaw += xoffset;
            //pitch += yoffset;
            //
            //if (constrainPitch)
            //{
            //    if (pitch > 89.0f)
            //        pitch = 89.0f;
            //    if (pitch < -89.0f)
            //        pitch = -89.0f;
            //}
            //updateCameraVectors();
        };
    }

}


