#include "pch.h"
#include "Camera.h"



namespace cy3d
{
    Camera::Camera(VulkanContext& context, m3d::Vec3f pos, m3d::Vec3f wUp, float yaw0, float pitch0) : _context(context), pos(pos), worldUp(wUp), yaw(yaw0), pitch(pitch0) {}

    Camera::~Camera()
    {
        //clean up listeners
        _context.getWindow()->removeKeyboardListener(keyboardInputListenerId);
        _context.getWindow()->removeMouseListener(mouseInputListenerId);
    }

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

    Camera* Camera::create3D(VulkanContext& context, float fov, float width, float height, float far, float near)
    {
        Camera* camera = new Camera(context);
        camera->projectionMatrix = createPerspectiveMatrix(fov, width / height, far, near);

        camera->keyboardInputListenerId = context.getWindow()->registerKeyboardListener
        (
            [cam = camera](KeyBoardInputEvent& e, double deltaTime)
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
            }
        );

        camera->mouseInputListenerId = context.getWindow()->registerMouseListener
        (
            [cam = camera, width, height](MouseInputEvent& e, double deltaTime)
            {
                cam->lastX = width / 2.0f;
                cam->lastY = height / 2.0f;
                if (cam->isFirstMouse)
                {
                    cam->lastX = e.xpos;
                    cam->lastY = e.ypos;
                    cam->isFirstMouse = false;
                }

                float xoffset = e.xpos - cam->lastX;
                float yoffset = cam->lastY - e.ypos; // reversed since y-coordinates go from bottom to top

                cam->lastX = e.xpos;
                cam->lastY = e.ypos;

                xoffset *= cam->mouseSensitivity;
                yoffset *= cam->mouseSensitivity;
                
                cam->yaw += xoffset;
                cam->pitch += yoffset;
                
                if (cam->pitch > 89.0f)
                    cam->pitch = 89.0f;
                if (cam->pitch < -89.0f)
                    cam->pitch = -89.0f;
                
                float rYaw = m3d::degreesToRadians(cam->yaw);
                float rPitch = m3d::degreesToRadians(cam->pitch);
                m3d::Vec3f cNewLookDir;
                cNewLookDir.x() = std::cos(rYaw) * std::cos(rPitch);
                cNewLookDir.y() = std::sin(rPitch);
                cNewLookDir.z() = std::sin(rYaw) * std::cos(rPitch);

                cam->lookDir = cNewLookDir;
                cam->right = m3d::Vec3f::cross(cam->lookDir, cam->worldUp);
                cam->cUp = m3d::Vec3f::cross(cam->right, cam->lookDir);

                cam->lookDir.normalized();
                cam->right.normalized();
                cam->cUp.normalized();
            }
        );

        return camera;
    }

}


