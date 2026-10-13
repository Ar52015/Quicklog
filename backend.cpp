#include "backend.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QTime>

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

QString Backend::formatBullet(const QString& text) const {
    QString time = QTime::currentTime().toString("hh:mm AP");
    return "- \\[ " + time + " \\] - " + text;
}
