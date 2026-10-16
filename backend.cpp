#include "backend.h"
#include "fronts.h"

#include <QDate>
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

bool Backend::logExists() const {
    if (vaultFolder().isEmpty()) {
        return false;
    }
    QString name = QDate::currentDate().toString("yyyy-MM-dd") + ".md";
    return QFile::exists(QDir(vaultFolder()).filePath("Logs/" + name));
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
    QDir logDir(QDir(vaultFolder()).filePath("Logs"));
    QMap<QString, QString> logs;
    for (const QFileInfo& file : logDir.entryInfoList({"*.md"}, QDir::Files)) {
        QFile log(file.filePath());
        if (log.open(QIODevice::ReadOnly)) {
            logs.insert(file.completeBaseName(), readText(file.filePath()));
        }
    }
    return orderFronts(names, logs);
}

void Backend::refresh() {
    emit logExistsChanged();
    emit frontsChanged();
}

QString Backend::formatBullet(const QString& text) const {
    QString time = QTime::currentTime().toString("hh:mm AP");
    return "- \\[ " + time + " \\] - " + text;
}
