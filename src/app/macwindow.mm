#include "macwindow.h"
#include "backend.h"

#include <QCursor>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QScreen>
#include <QWindow>
#include <bit>

#import <AppKit/AppKit.h>

static void floatOverFullScreen(QWindow* window) {
    NSWindow* nsWindow = std::bit_cast<NSView*>(window->winId()).window;
    nsWindow.collectionBehavior = NSWindowCollectionBehaviorMoveToActiveSpace |
                                  NSWindowCollectionBehaviorFullScreenAuxiliary;
    nsWindow.level = NSFloatingWindowLevel;
}

void loadWindow(QQmlApplicationEngine& engine) {
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
    if (Backend().vaultFolder().isEmpty()) {
        window->setPosition(area.x() + (area.width() - window->width()) / 2,
                            area.y() + (area.height() - window->height()) / 2);
    } else {
        window->setPosition(area.x() + area.width() - window->width() - gap,
                            area.y() + area.height() - window->height() - gap);
    }
    floatOverFullScreen(window);
    window->show();
}
