#include "backend.h"

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
}

void Backend::setVaultFolderUrl(const QUrl& folder) {
    setVaultFolder(folder.toLocalFile());
}

QString Backend::formatBullet(const QString& text) const {
    QString time = QTime::currentTime().toString("hh:mm AP");
    return "- \\[ " + time + " \\] - " + text;
}
