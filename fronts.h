#ifndef FRONTS_H
#define FRONTS_H

#include <QMap>
#include <QString>
#include <QStringList>

QStringList orderFronts(const QStringList& fronts,
                        const QMap<QString, QString>& logs);
bool isActiveProject(const QString& projectText);

#endif
