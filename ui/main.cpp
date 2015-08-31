#include "ui/MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName("SofaFrontEndViewer");
	app.setOrganizationName("Christophe Guebert");

	MainWindow window;
	window.show();

	return app.exec();
}
