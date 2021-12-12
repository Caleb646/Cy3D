#pragma once
#include "pch.h"

#include "src/platform/Vulkan/VulkanWindow.h"
#include "src/platform/Vulkan/VulkanContext.h"

namespace cy3d
{
    constexpr float YAW = -90.0f;
    constexpr float PITCH = 0.0f;
    constexpr float SPEED = 30.0f;
    constexpr float SENSITIVITY = 0.1f;
    constexpr float ZOOM = 45.0f;

    class Camera
    {
    public:

        enum class eDirs
        {
            FORWARD,
            BACKWARD,
            LEFT,
            RIGHT
        };


        float yaw = YAW;
        float pitch = PITCH;
        float moveSpeed = SPEED;
        float mouseSensitivity = SENSITIVITY;
        float zoom = ZOOM;

        float lastX{};
        float lastY{};

         m3d::vec3f pos{};
         m3d::vec3f lookDir{};
         m3d::vec3f cUp{};
         m3d::vec3f right{};
         const m3d::vec3f worldUp{};

         m3d::mat4f projectionMatrix;

         VulkanWindow::listener_id keyboardInputListenerId;
         VulkanWindow::listener_id mouseInputListenerId;

         VulkanContext& _context;

         bool isFirstMouse{ true };

    public:
        Camera(VulkanContext& context, m3d::vec3f pos = m3d::vec3f(2.0f, 2.0f, 2.0f), m3d::vec3f wUp = m3d::vec3f(0.0f, -1.0f, 0.0f), float yaw0 = YAW, float pitch0 = PITCH);
        ~Camera();

        void print();
        void updateVectors();
        m3d::mat4f getLookAt();

        static m3d::mat4f createLookAtMatrix(m3d::vec3f pos, m3d::vec3f lookDir, m3d::vec3f cUp);
        static m3d::mat4f createLookAtMatrix(Camera& camera);
        static m3d::mat4f createPerspectiveMatrix(float fov, float aspectRatio, float far, float near);

        static Camera* create3D(VulkanContext& context, float fov, float width, float height, float far, float near);

    };
}

