#ifndef LOGFILE_H
#define LOGFILE_H

#include <QDate>
#include <QString>
#include <optional>

std::optional<QString> addToLog(const QString& path, const QString& front,
                                const QString& phase, const QString& bullet);
std::optional<QString> addPhaseToProject(const QString& path,
                                         const QString& name, const QDate& day);

#endif
