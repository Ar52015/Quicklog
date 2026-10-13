#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

class Backend : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString vaultFolder READ vaultFolder WRITE setVaultFolder NOTIFY
                   vaultFolderChanged)

  public:
    QString vaultFolder() const;
    void setVaultFolder(const QString& folder);

    Q_INVOKABLE QString formatBullet(const QString& text) const;

  signals:
    void vaultFolderChanged();
};

#endif
