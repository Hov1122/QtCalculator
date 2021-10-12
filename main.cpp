#include "calculator.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Calculator w;
    w.setWindowIcon(QIcon(":/pre/images/calculator-icon.png"));
    w.show();
    return a.exec();
}
