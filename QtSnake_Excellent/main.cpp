// main.cpp：创建应用和主窗口，启动 Qt 事件循环。
#include "widget.h"
#include <QApplication>
#include <QCoreApplication>
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    // 组织名和应用名用于确定应用数据目录，存放排行榜与存档。
    QCoreApplication::setOrganizationName(QStringLiteral("SnakeLab"));
    QCoreApplication::setApplicationName(QStringLiteral("QtSnakeTraining"));
    Widget window;
    window.show();
    // 事件循环分发按钮、键盘、定时器和重绘事件。
    return app.exec();
}
