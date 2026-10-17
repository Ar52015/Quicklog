#ifndef LOGEDIT_H
#define LOGEDIT_H

#include <QString>
#include <optional>

QString withFront(const QString& log, const QString& front);
std::optional<QString> withTrack(const QString& log, const QString& front,
                                 const QString& phase);
std::optional<QString> withBullet(const QString& log, const QString& front,
                                  const QString& bullet);

#endif
