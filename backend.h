#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

class Backend : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString vaultFolder READ vaultFolder WRITE setVaultFolder NOTIFY
                   vaultFolderChanged)
    Q_PROPERTY(bool logExists READ logExists NOTIFY logExistsChanged)
    Q_PROPERTY(QStringList fronts READ fronts NOTIFY frontsChanged)

  public:
    QString vaultFolder() const;
    QStringList fronts() const;

    void setVaultFolder(const QString& folder);

    bool logExists() const;

    Q_INVOKABLE void setVaultFolderUrl(const QUrl& folder);
    Q_INVOKABLE void refresh();

    Q_INVOKABLE QString formatBullet(const QString& text) const;

  signals:
    void vaultFolderChanged();
    void logExistsChanged();
    void frontsChanged();
};

#endif
