#include "BSPReaderWin.hh"

#include <QApplication>

int main(int argc, char * * argv) {
	
	QApplication app { argc, argv };
	
	BSPReaderWindow * win = new BSPReaderWindow;
	win->show();
	
	return app.exec();
}
