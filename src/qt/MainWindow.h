#pragma once

#include <QApplication>
#include <QWidget>

class Emulator;
class RenderWindow;

class MainWindow : public QWidget {

public:
	MainWindow();
	~MainWindow();

private:
	Emulator* emu;
	RenderWindow* renderWindow;
	QWidget* renderWidget;

protected:
	void closeEvent(QCloseEvent* event) override;
};