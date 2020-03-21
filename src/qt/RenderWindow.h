#pragma once

#include <vulkan/vulkan.hpp>

#include <QWindow>
#include <QVulkanInstance>

class Emulator;

class RenderWindow : public QWindow {

public:
	bool rendering = false;

	RenderWindow(Emulator* emu);

	void renderLater();
	void renderNow();
	void release();

protected:
	bool initialised = false;
	QVulkanInstance inst;
	Emulator* emu;

	bool event(QEvent *event) override;
	void exposeEvent(QExposeEvent *event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

public:
	void init();
	void handleKey(int keyCode, bool isPressed);
};