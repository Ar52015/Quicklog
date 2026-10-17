#ifndef LOGFILE_H
#define LOGFILE_H

#include <QString>
#include <optional>

std::optional<QString> addToLog(const QString& path, const QString& front,
                                const QString& phase, const QString& bullet);

#endif
