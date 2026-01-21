#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication a(argc, argv);

    QFont font("Segoe UI", 9);
    if (font.exactMatch()) {
        a.setFont(font);
    } else {
        a.setFont(QFont("Microsoft YaHei UI", 9));
    }

    MainWindow w;
    w.show();
    return a.exec();
}
