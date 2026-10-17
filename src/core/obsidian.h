#ifndef OBSIDIAN_H
#define OBSIDIAN_H

#include <QString>
#include <QUrl>

QUrl obsidianOpenUrl(const QString& notePath);
QUrl obsidianDailyUrl(const QString& vaultName);
QString vaultFolderFor(const QString& chosen);

#endif
