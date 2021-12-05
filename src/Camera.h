#pragma once

#include "src/platform/Vulkan/VulkanWindow.h"
#include "src/platform/Vulkan/VulkanContext.h"

#include <M3D/M3D.h>

namespace cy3d
{
    constexpr float YAW = -90.0f;
    constexpr float PITCH = 0.0f;
    constexpr float SPEED = 2.5f;
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

         m3d::Vec3f pos;
         m3d::Vec3f lookDir;
         m3d::Vec3f cUp;
         m3d::Vec3f right;
         m3d::Vec3f worldUp;

         m3d::Mat4f projectionMatrix;

         VulkanWindow::listener_id keyboardInputListenerId;
         VulkanWindow::listener_id mouseInputListenerId;

         VulkanContext& _context;

         bool isFirstMouse{ false };

    public:
        Camera(VulkanContext& context, m3d::Vec3f pos = m3d::Vec3f(0.0f, 0.0f, 0.0f), m3d::Vec3f wUp = m3d::Vec3f(0.0f, 1.0f, 0.0f), float yaw0 = YAW, float pitch0 = PITCH);
        ~Camera();

        void print();

        static m3d::Mat4f createLookAtMatrix(m3d::Vec3f pos, m3d::Vec3f lookDir, m3d::Vec3f cUp);
        static m3d::Mat4f createLookAtMatrix(Camera& camera);
        static m3d::Mat4f createPerspectiveMatrix(float fov, float aspectRatio, float far, float near);

        static Camera* create3D(VulkanContext& context, float fov, float width, float height, float far, float near);
    };
}

