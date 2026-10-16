#include "fronts.h"

#include <QHash>
#include <QRegularExpression>
#include <algorithm>

static QString normFront(QString cell) {
    static const QHash<QString, QString> alias = {
        {"nemoclaw-codecious", "Nemoclaw-Codeacious"},
        {"interview prep", "Interview-Prep"},
    };
    cell = cell.trimmed();
    cell.remove(QRegularExpression("^\\*+|\\*+$"));
    cell.remove(QRegularExpression(":$"));
    cell = cell.trimmed();
    cell.replace("\\[", "[").replace("\\]", "]");
    cell.remove(QRegularExpression("\\s*\\[.*?\\]\\s*"));
    cell = cell.trimmed();
    return alias.value(cell.toLower(), cell);
}

QStringList orderFronts(const QStringList& fronts,
                        const QMap<QString, QString>& logs) {
    QHash<QString, QString> lastDate;
    QStringList seen;
    for (const QString& date : logs.keys()) {
        QString text = logs.value(date);
        text.remove(QRegularExpression(
            "<!--.*?-->", QRegularExpression::DotMatchesEverythingOption));
        for (const QString& line : text.split('\n')) {
            QString row = line.trimmed();
            if (!row.startsWith('|')) {
                continue;
            }
            if (row.endsWith('|')) {
                row.chop(1);
            }
            QStringList cells = row.mid(1).split('|');
            if (cells.size() < 3) {
                continue;
            }
            QString first = cells[0].trimmed();
            if (first.contains(QRegularExpression("^:?-{2,}")) ||
                first.toLower() == "front") {
                continue;
            }
            QString front = normFront(first);
            if (front.isEmpty()) {
                continue;
            }
            if (!lastDate.contains(front)) {
                seen.append(front);
            }
            lastDate.insert(front, date);
        }
    }

    QStringList withRows;
    for (const QString& front : seen) {
        if (fronts.contains(front)) {
            withRows.append(front);
        }
    }
    std::stable_sort(withRows.begin(), withRows.end(),
                     [&](const QString& a, const QString& b) {
                         return lastDate.value(a) > lastDate.value(b);
                     });

    QStringList withoutRows;
    for (const QString& front : fronts) {
        if (!lastDate.contains(front)) {
            withoutRows.append(front);
        }
    }
    withoutRows.sort(Qt::CaseInsensitive);

    return withRows + withoutRows;
}

bool isActiveProject(const QString& projectText) {
    for (const QString& line : projectText.split('\n')) {
        if (line.startsWith("Status:")) {
            QString status = line.mid(7).trimmed().toLower();
            return status != "done" && status != "abandoned";
        }
    }
    return true;
}
