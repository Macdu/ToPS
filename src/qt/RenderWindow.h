#pragma once

#include <vulkan/vulkan.hpp>

#include <QWindow>
#include <QVulkanInstance>

class Emulator;

class RenderWindow : public QWindow {
	Q_OBJECT

public:
	bool rendering = false;

	RenderWindow(Emulator* emu);

public slots:
	void renderLater();
	void renderNow();
	void release();

protected:
	bool initialised = false;
	QVulkanInstance inst;
	Emulator* emu;

	bool event(QEvent *event) override;
	void exposeEvent(QExposeEvent *event) override;
public:
	void init();
};