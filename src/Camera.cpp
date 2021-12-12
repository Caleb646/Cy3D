#include "pch.h"
#include "Camera.h"



namespace cy3d
{
    Camera::Camera(VulkanContext& context, m3d::vec3f pos, m3d::vec3f wUp, float yaw0, float pitch0) : _context(context), pos(pos), worldUp(wUp), yaw(yaw0), pitch(pitch0)
    {
        updateVectors();
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

    void Camera::updateVectors()
    {
        float rYaw = m3d::toRadians(yaw);
        float rPitch = m3d::toRadians(pitch);
        m3d::vec3f cNewLookDir;
        cNewLookDir.x() = std::cos(rYaw) * std::cos(rPitch);
        cNewLookDir.y() = std::sin(rPitch);
        cNewLookDir.z() = std::sin(rYaw) * std::cos(rPitch);

        lookDir = m3d::normalize(cNewLookDir); //m3d::vec3f(0.0f, 0.0f, 1.0f); //;
        right = m3d::normalize(m3d::cross(lookDir, worldUp));
        cUp = m3d::normalize(m3d::cross(right, lookDir));
    }

    m3d::mat4f Camera::getLookAt()
    {
        return m3d::lookAt(pos, pos + lookDir, cUp);
    }


    /**
    * 
    * 
    * Public Static Methods
    * 
    * 
    */
    m3d::mat4f  Camera::createLookAtMatrix(m3d::vec3f pos, m3d::vec3f lookDir, m3d::vec3f cUp)
    {
        return m3d::lookAt(pos, pos + lookDir, cUp);
    }

    m3d::mat4f  Camera::createLookAtMatrix(Camera& camera)
    {
        return m3d::lookAt(camera.pos, camera.pos + camera.lookDir, camera.cUp);
    }

    m3d::mat4f  Camera::createPerspectiveMatrix(float fov, float aspectRatio, float far, float near)
    {
        return m3d::perspective(m3d::toRadians(fov), aspectRatio, far, near);
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
                    cam->pos -= cam->right * velocity;
                }
                else if (e.key == GLFW_KEY_D && (e.action == GLFW_PRESS || e.action == GLFW_REPEAT))
                {
                    cam->pos += cam->right * velocity;
                }               
            }
        );

        camera->mouseInputListenerId = context.getWindow()->registerMouseListener
        (
            //TODO input events should store the window width and height
            //and some kind of void* userData that could be cast back.
            //such as static_cast<Camera*>(void* userData);
            //this would remove the need for std::function
            [cam = camera, width, height](MouseInputEvent& e, double deltaTime)
            {
                if (cam->isFirstMouse)
                {
                    cam->lastX = width * 0.5f;
                    cam->lastY = height * 0.5f;
                    cam->isFirstMouse = false;
                }

                float xoffset = e.xpos - cam->lastX;
                float yoffset = e.ypos - cam->lastY;

                cam->lastX = e.xpos;
                cam->lastY = e.ypos;

                xoffset *= cam->mouseSensitivity;
                yoffset *= cam->mouseSensitivity;
                
                cam->yaw -= xoffset;
                cam->pitch += yoffset;
                
                if (cam->pitch > 89.0f)
                {
                    cam->pitch = 89.0f;
                }

                if (cam->pitch < -89.0f)
                {
                    cam->pitch = -89.0f;
                }
       
                cam->updateVectors();
            }
        );

        return camera;
    }

}


