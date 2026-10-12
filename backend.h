#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

class Backend : public QObject {
    Q_OBJECT
    QML_ELEMENT

  public:
    Q_INVOKABLE QString formatBullet(const QString& text) const;
};

#endif
