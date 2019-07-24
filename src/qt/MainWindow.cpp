#include "MainWindow.h"

#include "../Emulator.h"
#include "RenderWindow.h"

MainWindow::MainWindow() : QWidget()
{
	setFixedSize(1024, 512);
	emu = new Emulator();
	renderWindow = new RenderWindow(emu);
	renderWidget = QWidget::createWindowContainer(renderWindow, this);
	renderWidget->setFixedSize(1024, 512);
}

MainWindow::~MainWindow()
{
	delete emu;
	delete renderWindow;
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	// clean the vulkan instance
	emu->destroy();
	renderWindow->rendering = false;

	QWidget::closeEvent(event);
}
