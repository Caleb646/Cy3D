#include "pch.h"

#include "FirstApp.h"

#include <Logi/Logi.h>

namespace cy3d
{

	FirstApp::FirstApp() 
	{
		VulkanContext::createDefaultContext(cyContext);
		sceneRenderer.reset(new SceneRenderer(cyContext));
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run()
	{
		float deltaTime = 0.0f;
		float lastFrame = 0.0f;

		camera.reset(Camera::create3D(cyContext, 90.0f, cyContext.getWindowWidth(), cyContext.getWindowHeight(), 0.1f, 100.0f));

		//VulkanShader shader(cyContext, "resources/shaders");

		while (!cyContext.getWindow()->shouldClose())
		{
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;
			cyContext.getWindow()->processInput(deltaTime);
			drawFrame();
		}
		vkDeviceWaitIdle(cyContext.getDevice()->device());
	}

	/**
	 * @brief The drawFrame function will perform the following operations:
	 *
	 * Acquire an image from the swap chain
	 * Execute the command buffer with that image as attachment in the framebuffer
	 * Return the image to the swap chain for presentation
	 * 
	 * Each of these events is set in motion using a single function call, but they are executed asynchronously. 
	 * The function calls will return before the operations are actually finished and the order of execution is also undefined.
	*/
	void FirstApp::drawFrame()
	{
		sceneRenderer->beginScene(camera);
		sceneRenderer->endScene();
		/*cyContext.getVulkanRenderer()->beginFrame();
		cyContext.getVulkanRenderer()->beginRenderPass();
		cyContext.getVulkanRenderer()->endRenderPass();
		cyContext.getVulkanRenderer()->endFrame();*/


	}
}



