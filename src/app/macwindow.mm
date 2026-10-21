#include "macwindow.h"

#include <QCursor>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QScreen>
#include <QWindow>

void loadInCorner(QQmlApplicationEngine& engine) {
    engine.setInitialProperties({{"visible", false}});
    engine.loadFromModule("Quicklog", "Main");
    auto* window = qobject_cast<QWindow*>(engine.rootObjects().value(0));
    if (window == nullptr) {
        return;
    }

    const int gap = 30;
    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    if (screen == nullptr) {
        screen = QGuiApplication::primaryScreen();
    }
    const QRect area = screen->availableGeometry();
    window->setPosition(area.x() + area.width() - window->width() - gap,
                        area.y() + area.height() - window->height() - gap);
    window->show();
}
