#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    app.setOrganizationName("quicklog");
#ifdef QUICKLOG_DEV
    app.setApplicationName("quicklog-dev");
#endif

    QQmlApplicationEngine engine;
    engine.loadFromModule("Quicklog", "Main");

    return app.exec();
}
