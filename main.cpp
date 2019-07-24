
#include <QApplication>
#include <QLoggingCategory>
#include <cstdio>

#include "src/qt/MainWindow.h"

int main(int argc, char *argv[])
{
	freopen("../psxdebug.txt", "w", stdout);
	// enables vulkan debugging
	QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));
    QApplication app(argc, argv);

	MainWindow window;
	window.show();

    int res = app.exec();
	return res;
}
