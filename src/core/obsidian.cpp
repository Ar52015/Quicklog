#include "obsidian.h"

#include <QDir>
#include <QRegularExpression>

QUrl obsidianOpenUrl(const QString& notePath) {
    QString note = notePath;
    if (note.endsWith(".md")) {
        note.chop(3);
    }
    return QUrl::fromEncoded("obsidian://open?path=" +
                             QUrl::toPercentEncoding(note));
}

QUrl obsidianDailyUrl(const QString& vaultName) {
    return QUrl::fromEncoded("obsidian://daily?vault=" +
                             QUrl::toPercentEncoding(vaultName));
}

QString vaultFolderFor(const QString& chosen) {
    QDir dir(chosen);
    if (dir.exists(".obsidian")) {
        return dir.absolutePath();
    }
    if (dir.cdUp() && dir.exists(".obsidian")) {
        return dir.absolutePath();
    }
    return {};
}

QString wikilinksAsLinks(QString markdown) {
    static const QRegularExpression aliased(
        R"((?<!!)\[\[[^\]|]+\|([^\]]+)\]\])");
    static const QRegularExpression plain(R"((?<!!)\[\[([^\]|]+)\]\])");
    return markdown.replace(aliased, "[\\1]()").replace(plain, "[\\1]()");
}
