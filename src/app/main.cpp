#include <QColor>
#include <QGuiApplication>
#include <QPalette>
#include <QQmlApplicationEngine>

#ifdef Q_OS_MACOS
#include "macwindow.h"
#endif

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    app.setOrganizationName("quicklog");
#ifdef QUICKLOG_DEV
    app.setApplicationName("quicklog-dev");
    app.setDesktopFileName("quicklog-dev");
#endif

    const QColor rosePineIris(0xc4a7e7);
    QPalette palette = QGuiApplication::palette();
    palette.setColor(QPalette::Link, rosePineIris);
    QGuiApplication::setPalette(palette);

    QQmlApplicationEngine engine;
#ifdef Q_OS_MACOS
    loadInCorner(engine);
#else
    engine.loadFromModule("Quicklog", "Main");
#endif

    return app.exec();
}
