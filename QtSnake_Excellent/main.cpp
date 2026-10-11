// main.cpp - Qt Widgets application entry.
#include "widget.h"
#include <QApplication>
#include <QCoreApplication>
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("SnakeLab"));
    QCoreApplication::setApplicationName(QStringLiteral("QtSnakeTraining"));
    Widget window;
    window.show();
    return app.exec();
}
