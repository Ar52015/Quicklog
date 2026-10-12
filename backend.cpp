#include "backend.h"

#include <QTime>

QString Backend::formatBullet(const QString& text) const {
    QString time = QTime::currentTime().toString("hh:mm AP");
    return "- \\[ " + time + " \\] - " + text;
}
