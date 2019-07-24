#include "RenderWindow.h"

#include "../Emulator.h"

RenderWindow::RenderWindow(Emulator* emu) {
	this->emu = emu;
}

void RenderWindow::renderLater() {
	requestUpdate();
}

void RenderWindow::renderNow() {
	if (!rendering)return;
	emu->renderFrame();
	if (isExposed()) {
		renderLater();
	}
}

void RenderWindow::release() {
	rendering = false;
	emu->destroy();
}

bool RenderWindow::event(QEvent *event) {
	switch (event->type())
	{
	case QEvent::UpdateRequest:
		renderNow();
		return true;
	default:
		return QWindow::event(event);
	}
}

void RenderWindow::exposeEvent(QExposeEvent *event) {
	Q_UNUSED(event);

	if (!initialised) {
		init();
	}

	renderNow();
}

void RenderWindow::init()
{
	initialised = true;

	// instance creation
#ifndef NDEBUG
	inst.setLayers(QByteArrayList() << "VK_LAYER_LUNARG_standard_validation");
#endif
	inst.create();
	setVulkanInstance(&inst);

	setSurfaceType(QWindow::VulkanSurface);
	vk::SurfaceKHR surface = static_cast<vk::SurfaceKHR>(QVulkanInstance::surfaceForWindow(this));
	// initializes the gpu
	emu->init(this,static_cast<vk::Instance>(inst.vkInstance()), surface);

	rendering = true;
}