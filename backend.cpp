#include "backend.h"
#include "fronts.h"
#include "logedit.h"
#include "logfile.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QTime>

static QString readText(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

QString Backend::vaultFolder() const {
    return QSettings().value("vaultFolder").toString();
}

void Backend::setVaultFolder(const QString& folder) {
    if (folder == vaultFolder()) {
        return;
    }
    QSettings().setValue("vaultFolder", folder);
    emit vaultFolderChanged();
    emit logExistsChanged();
    emit frontsChanged();
}

void Backend::setVaultFolderUrl(const QUrl& folder) {
    setVaultFolder(folder.toLocalFile());
}

QString Backend::todayLogPath() const {
    QString name =
        logDay(QDateTime::currentDateTime()).toString("yyyy-MM-dd") + ".md";
    return QDir(vaultFolder()).filePath("Logs/" + name);
}

bool Backend::logExists() const {
    if (vaultFolder().isEmpty()) {
        return false;
    }
    return QFile::exists(todayLogPath());
}

QStringList Backend::fronts() const {
    if (vaultFolder().isEmpty()) {
        return {};
    }
    QDir projects(QDir(vaultFolder()).filePath("Projects"));
    QStringList names;
    for (const QFileInfo& file :
         projects.entryInfoList({"*.md"}, QDir::Files, QDir::Name)) {
        if (isActiveProject(readText(file.filePath()))) {
            names.append(file.completeBaseName());
        }
    }
    return orderFronts(names, readLogs());
}

QString Backend::defaultFront() const {
    return chooseDefaultFront(fronts(), readLogs());
}

QMap<QString, QString> Backend::readLogs() const {
    QDir logDir(QDir(vaultFolder()).filePath("Logs"));
    QMap<QString, QString> logs;
    for (const QFileInfo& file : logDir.entryInfoList({"*.md"}, QDir::Files)) {
        logs.insert(file.completeBaseName(), readText(file.filePath()));
    }
    return logs;
}

QString Backend::currentFront() const { return m_currentFront; }

void Backend::setCurrentFront(const QString& front) {
    if (front == m_currentFront) {
        return;
    }
    m_currentFront = front;
    emit currentFrontChanged();
    emit phasesChanged();
}

QStringList Backend::phases() const {
    if (vaultFolder().isEmpty() || m_currentFront.isEmpty()) {
        return {};
    }
    QDir projects(QDir(vaultFolder()).filePath("Projects"));
    return inProgressPhases(
        readText(projects.filePath(m_currentFront + ".md")));
}

QString Backend::defaultPhase() const {
    return chooseDefaultPhase(phases(), readText(todayLogPath()),
                              m_currentFront);
}

void Backend::refresh() {
    emit logExistsChanged();
    emit frontsChanged();
    emit phasesChanged();
}

void Backend::addEntry(const QString& front, const QString& phase,
                       const QString& type, const QString& text) {
    if (!logExists()) {
        emit entryFailed("Today's log doesn't exist yet. Create it in "
                         "Obsidian, then press ↻.");
        return;
    }
    if (front.isEmpty()) {
        emit entryFailed("There's no front to add this to.");
        return;
    }
    QString bullet = text.trimmed().isEmpty()
                         ? QString()
                         : formatBullet(type, text.trimmed());
    if (std::optional<QString> error = addToLog(
            todayLogPath(), front, phase.isEmpty() ? "-" : phase, bullet)) {
        emit entryFailed(*error);
        return;
    }
    emit entryAdded();
}

QString Backend::formatBullet(const QString& type, const QString& text) const {
    return bulletLine(QTime::currentTime(), type, text);
}
