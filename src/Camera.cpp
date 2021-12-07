#include "pch.h"
#include "Camera.h"



namespace cy3d
{
    Camera::Camera(VulkanContext& context, m3d::Vec3f pos, m3d::Vec3f wUp, float yaw0, float pitch0) : _context(context), pos(pos), worldUp(wUp), yaw(yaw0), pitch(pitch0)
    {
        float rYaw = m3d::degreesToRadians(yaw);
        float rPitch = m3d::degreesToRadians(pitch);
        m3d::Vec3f cNewLookDir;
        cNewLookDir.x() = std::cos(rYaw) * std::cos(rPitch);
        cNewLookDir.y() = std::sin(rPitch);
        cNewLookDir.z() = std::sin(rYaw) * std::cos(rPitch);

        lookDir = cNewLookDir;
        right = m3d::Vec3f::cross(lookDir, worldUp);
        cUp = m3d::Vec3f::cross(right, lookDir);

        lookDir.normalized();
        right.normalized();
        cUp.normalized();
    }

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

        //std::cout << "Look" << std::endl;
        //lookDir.print();
        //std::cout << std::endl;

        //std::cout << "Right" << std::endl;
        //right.print();
        //std::cout << std::endl;
    }

    m3d::Mat4f Camera::getLookAt()
    {
        //return m3d::Mat4f::getLookAt(pos, pos + lookDir, cUp);

        m3d::Vec3f f((pos + lookDir) - pos);
        f.normalized();
        m3d::Vec3f s;
        s = m3d::Vec3f::cross(f, cUp);
        s.normalized();
        m3d::Vec3f u;
        u = m3d::Vec3f::cross(s, f);

        m3d::Mat4f out;
        out(0, 0) = s.x();
        out(0, 1) = s.y();
        out(0, 2) = s.z();
        out(1, 0) = u.x();
        out(1, 1) = u.y();
        out(1, 2) = u.z();
        out(2, 0) = f.x();
        out(2, 1) = f.y();
        out(2, 2) = -f.z();

        out(0, 3) = -m3d::Vec3f::dot(s, pos);
        out(1, 3) = -m3d::Vec3f::dot(u, pos);
        out(2, 3) = m3d::Vec3f::dot(f, pos);

        return out;
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
        camera->projectionMatrix = createPerspectiveMatrix(fov, height / width, far, near);

        camera->keyboardInputListenerId = context.getWindow()->registerKeyboardListener
        (
            [cam = camera](KeyBoardInputEvent& e, double deltaTime)
            {
                float velocity = cam->moveSpeed * deltaTime;
                if (e.key == GLFW_KEY_W && (e.action == GLFW_PRESS || e.action == GLFW_REPEAT))
                {
                    cam->pos += cam->lookDir * velocity;
                }
                else if (e.key == GLFW_KEY_S && (e.action == GLFW_PRESS || e.action == GLFW_REPEAT))
                {
                    cam->pos -= cam->lookDir * velocity;
                }
                else if (e.key == GLFW_KEY_A && (e.action == GLFW_PRESS || e.action == GLFW_REPEAT))
                {
                    cam->pos += cam->right * velocity;
                }
                else if (e.key == GLFW_KEY_D && (e.action == GLFW_PRESS || e.action == GLFW_REPEAT))
                {
                    cam->pos -= cam->right * velocity;
                }               
            }
        );

        camera->mouseInputListenerId = context.getWindow()->registerMouseListener
        (
            [cam = camera, width, height](MouseInputEvent& e, double deltaTime)
            {
                //cam->lastX = width / 2.0f;
                //cam->lastY = height / 2.0f;
                //if (cam->isFirstMouse)
                //{
                //    cam->lastX = e.xpos;
                //    cam->lastY = e.ypos;
                //    cam->isFirstMouse = false;
                //}

                //float xoffset = e.xpos - cam->lastX;
                //float yoffset = cam->lastY - e.ypos; // reversed since y-coordinates go from bottom to top

                //cam->lastX = e.xpos;
                //cam->lastY = e.ypos;

                //xoffset *= cam->mouseSensitivity;
                //yoffset *= cam->mouseSensitivity;
                //
                //cam->yaw += xoffset;
                //cam->pitch += yoffset;
                //
                //if (cam->pitch > 89.0f)
                //    cam->pitch = 89.0f;
                //if (cam->pitch < -89.0f)
                //    cam->pitch = -89.0f;
                //
                //float rYaw = m3d::degreesToRadians(cam->yaw);
                //float rPitch = m3d::degreesToRadians(cam->pitch);
                //m3d::Vec3f cNewLookDir;
                //cNewLookDir.x() = std::cos(rYaw) * std::cos(rPitch);
                //cNewLookDir.y() = std::sin(rPitch);
                //cNewLookDir.z() = std::sin(rYaw) * std::cos(rPitch);

                //cam->lookDir = cNewLookDir;
                //cam->right = m3d::Vec3f::cross(cam->lookDir, cam->worldUp);
                //cam->cUp = m3d::Vec3f::cross(cam->right, cam->lookDir);

                //cam->lookDir.normalized();
                //cam->right.normalized();
                //cam->cUp.normalized();
            }
        );

        return camera;
    }

}


