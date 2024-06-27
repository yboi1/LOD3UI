#include "nighteffect.h"
#include <QApplication>

#if _MSC_VER >= 1600
#pragma  execution_character_set("utf-8")
#endif


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	NightEffect w;
	QIcon icon(".\\64.png");
	w.setWindowIcon(icon);

	w.setWindowTitle("实景增强工具");
	w.initWindow();

	//w.setBaseSize(1000, 360);
	//w.setFixedSize(800, 600);
	w.show();
	return a.exec();
}
