#include "obsidian.h"

QUrl obsidianOpenUrl(const QString& notePath) {
    QString note = notePath;
    if (note.endsWith(".md")) {
        note.chop(3);
    }
    return QUrl::fromEncoded("obsidian://open?path=" +
                             QUrl::toPercentEncoding(note));
}
