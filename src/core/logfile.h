#ifndef LOGFILE_H
#define LOGFILE_H

#include <QDate>
#include <QString>
#include <optional>

std::optional<QString> addToLog(const QString& path, const QString& front,
                                const QString& phase, const QString& bullet);
std::optional<QString> addPhaseToProject(const QString& path,
                                         const QString& name,
                                         const QString& approach,
                                         const QDate& day);
std::optional<QString> setPhaseStatusInProject(const QString& path,
                                               const QString& name,
                                               const QString& status,
                                               const QString& approach,
                                               const QDate& day);

#endif
