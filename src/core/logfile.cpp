#include "logfile.h"
#include "fronts.h"
#include "logedit.h"

#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

static std::optional<QString> readText(const QString& path,
                                       const QString& label, QString& text) {
    QFile in(path);
    if (!in.open(QIODevice::ReadOnly)) {
        return "Couldn't read " + label + ": " + in.errorString();
    }
    text = QString::fromUtf8(in.readAll());
    return std::nullopt;
}

static std::optional<QString>
writeText(const QString& path, const QString& label, const QString& text) {
    QSaveFile out(path);
    if (!out.open(QIODevice::WriteOnly)) {
        return "Couldn't write " + label + ": " + out.errorString();
    }
    out.write(text.toUtf8());
    if (!out.commit()) {
        return "Couldn't write " + label + ": " + out.errorString();
    }
    return std::nullopt;
}

std::optional<QString> addToLog(const QString& path, const QString& front,
                                const QString& phase, const QString& bullet) {
    QString original;
    if (std::optional<QString> error =
            readText(path, "today's log", original)) {
        return error;
    }

    if (original.contains("<%")) {
        return QString("Obsidian hasn't finished setting up today's log. "
                       "Try again in a moment.");
    }

    bool crlf = original.contains("\r\n");
    QString text = original;
    text.replace("\r\n", "\n");

    text = withFront(text, front);
    if (!phase.isEmpty()) {
        std::optional<QString> tracked = withTrack(text, front, phase);
        if (!tracked) {
            return QString("Today's log has no Tracks table.");
        }
        text = *tracked;
    }
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
    return writeText(path, "today's log", text);
}

std::optional<QString> addPhaseToProject(const QString& path,
                                         const QString& name,
                                         const QString& approach,
                                         const QDate& day) {
    const QString label = QFileInfo(path).fileName();
    QString original;
    if (std::optional<QString> error = readText(path, label, original)) {
        return error;
    }

    bool crlf = original.contains("\r\n");
    QString text = original;
    text.replace("\r\n", "\n");

    std::optional<QString> added = withPhase(text, name, approach, day);
    if (!added) {
        return label + " has no phase table.";
    }
    if (*added == text) {
        return name + " is already a phase in " + label + ".";
    }
    text = *added;
    if (crlf) {
        text.replace("\n", "\r\n");
    }
    return writeText(path, label, text);
}

std::optional<QString> setPhaseStatusInProject(const QString& path,
                                               const QString& name,
                                               const QString& status,
                                               const QString& approach,
                                               const QDate& day) {
    const QString label = QFileInfo(path).fileName();
    QString original;
    if (std::optional<QString> error = readText(path, label, original)) {
        return error;
    }

    bool crlf = original.contains("\r\n");
    QString text = original;
    text.replace("\r\n", "\n");

    std::optional<QString> finished =
        withPhaseStatus(text, name, status, approach, day);
    if (!finished) {
        return name + " isn't in the phase table of " + label + ".";
    }
    text = *finished;
    if (crlf) {
        text.replace("\n", "\r\n");
    }
    if (text == original) {
        return std::nullopt;
    }
    return writeText(path, label, text);
}
