#ifndef BACKEND_H
#define BACKEND_H

#include <QMap>
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
    Q_PROPERTY(QString defaultFront READ defaultFront NOTIFY frontsChanged)
    Q_PROPERTY(QString currentFront READ currentFront WRITE setCurrentFront
                   NOTIFY currentFrontChanged)
    Q_PROPERTY(QStringList phases READ phases NOTIFY phasesChanged)
    Q_PROPERTY(QString defaultPhase READ defaultPhase NOTIFY phasesChanged)

  public:
    QString vaultFolder() const;
    QStringList fronts() const;
    QString defaultFront() const;
    QString currentFront() const;
    QStringList phases() const;
    QString defaultPhase() const;

    void setVaultFolder(const QString& folder);
    void setCurrentFront(const QString& front);

    bool logExists() const;

    Q_INVOKABLE void setVaultFolderUrl(const QUrl& folder);
    Q_INVOKABLE void refresh();

    Q_INVOKABLE QString formatBullet(const QString& text) const;

  signals:
    void vaultFolderChanged();
    void logExistsChanged();
    void frontsChanged();
    void currentFrontChanged();
    void phasesChanged();

  private:
    QString todayLogPath() const;
    QMap<QString, QString> readLogs() const;

    QString m_currentFront;
};

#endif
