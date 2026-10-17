#ifndef FRONTS_H
#define FRONTS_H

#include <QDate>
#include <QDateTime>
#include <QMap>
#include <QString>
#include <QStringList>

QString normFront(QString cell);
QStringList tableCells(const QString& line);

QStringList orderFronts(const QStringList& fronts,
                        const QMap<QString, QString>& logs);
bool isActiveProject(const QString& projectText);
QStringList inProgressPhases(const QString& projectText);
QString chooseDefaultPhase(const QStringList& phases, const QString& todaysLog,
                           const QString& front);

QString frontNameError(const QString& name);
QDate logDay(const QDateTime& now);
QString chooseDefaultFront(const QStringList& fronts,
                           const QMap<QString, QString>& logs);

#endif
