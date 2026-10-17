#include "logfile.h"
#include "logedit.h"

#include <QFile>
#include <QSaveFile>

std::optional<QString> addToLog(const QString& path, const QString& front,
                                const QString& phase, const QString& bullet) {
    QFile in(path);
    if (!in.open(QIODevice::ReadOnly)) {
        return "Couldn't read today's log: " + in.errorString();
    }
    const QString original = QString::fromUtf8(in.readAll());
    in.close();

    if (original.contains("<%")) {
        return QString("Obsidian hasn't finished setting up today's log. "
                       "Try again in a moment.");
    }

    bool crlf = original.contains("\r\n");
    QString text = original;
    text.replace("\r\n", "\n");

    text = withFront(text, front);
    std::optional<QString> tracked = withTrack(text, front, phase);
    if (!tracked) {
        return QString("Today's log has no Tracks table.");
    }
    text = *tracked;
    if (!bullet.isEmpty()) {
        std::optional<QString> bulleted = withBullet(text, front, bullet);
        if (!bulleted) {
            return QString("Today's log has no \"What's done today\" section.");
        }
        text = *bulleted;
    }

    if (crlf) {
        text.replace("\n", "\r\n");
    }
    if (text == original) {
        return std::nullopt;
    }

    QSaveFile out(path);
    if (!out.open(QIODevice::WriteOnly)) {
        return "Couldn't write today's log: " + out.errorString();
    }
    out.write(text.toUtf8());
    if (!out.commit()) {
        return "Couldn't write today's log: " + out.errorString();
    }
    return std::nullopt;
}
