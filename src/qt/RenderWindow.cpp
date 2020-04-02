#include "RenderWindow.h"

#include <QKeyEvent>
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

void RenderWindow::keyPressEvent(QKeyEvent* event)
{
	if (event->isAutoRepeat())return;
	handleKey(event->key(), true);
	if (event->key() == Qt::Key_W)Debugging::interpreter = true;
}

void RenderWindow::keyReleaseEvent(QKeyEvent* event)
{
	if (event->isAutoRepeat())return;
	handleKey(event->key(), false);
}

const std::map<int, ControllerKey > keys = {
	{Qt::Key_B, ControllerKey::Select},
	{Qt::Key_N, ControllerKey::Start},
	{Qt::Key_R, ControllerKey::L1},
	{Qt::Key_F, ControllerKey::L2},
	{Qt::Key_P, ControllerKey::R1},
	{Qt::Key_M, ControllerKey::R2},
	{Qt::Key_Q, ControllerKey::Left},
	{Qt::Key_D, ControllerKey::Right},
	{Qt::Key_Z, ControllerKey::Up},
	{Qt::Key_S, ControllerKey::Down},
	{Qt::Key_Left, ControllerKey::Square},
	{Qt::Key_Right, ControllerKey::Circle},
	{Qt::Key_Up, ControllerKey::Triangle},
	{Qt::Key_Down, ControllerKey::Cross}
};

void RenderWindow::handleKey(int keyCode, bool isPressed)
{
	if (keys.count(keyCode) != 0) {
		emu->handleInput(keys.at(keyCode), isPressed);
	}
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