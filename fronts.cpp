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

static QStringList tableCells(const QString& line) {
    QString row = line.trimmed();
    if (!row.startsWith('|')) {
        return {};
    }
    if (row.endsWith('|')) {
        row.chop(1);
    }
    QStringList cells = row.mid(1).split('|');
    for (QString& cell : cells) {
        cell = cell.trimmed();
    }
    return cells;
}

static QList<QStringList> trackRows(QString text) {
    text.remove(QRegularExpression(
        "<!--.*?-->", QRegularExpression::DotMatchesEverythingOption));
    QList<QStringList> rows;
    for (const QString& line : text.split('\n')) {
        QStringList cells = tableCells(line);
        if (cells.size() < 3 ||
            cells[0].contains(QRegularExpression("^:?-{2,}")) ||
            cells[0].toLower() == "front") {
            continue;
        }
        rows.append(cells);
    }
    return rows;
}

QStringList orderFronts(const QStringList& fronts,
                        const QMap<QString, QString>& logs) {
    QHash<QString, QString> lastDate;
    QStringList seen;
    for (const QString& date : logs.keys()) {
        for (const QStringList& cells : trackRows(logs.value(date))) {
            QString front = normFront(cells[0]);
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

QStringList inProgressPhases(const QString& projectText) {
    QRegularExpression italic("^\\*([^*]+)\\*$");
    QStringList phases;
    qsizetype phaseColumn = -1;
    qsizetype statusColumn = -1;
    for (const QString& line : projectText.split('\n')) {
        QStringList cells = tableCells(line);
        if (phaseColumn < 0) {
            phaseColumn = cells.indexOf("**Phase**");
            statusColumn = cells.indexOf("Status");
            continue;
        }
        if (cells.isEmpty()) {
            break;
        }
        QRegularExpressionMatch match = italic.match(cells.value(phaseColumn));
        QString status = cells.value(statusColumn).toLower();
        if (match.hasMatch() && (status.isEmpty() || status == "in-progress")) {
            phases.append(match.captured(1).trimmed());
        }
    }
    return phases;
}

QString chooseDefaultPhase(const QStringList& phases, const QString& todaysLog,
                           const QString& front) {
    QString today;
    for (const QStringList& cells : trackRows(todaysLog)) {
        if (normFront(cells[0]) == front) {
            today = cells[1];
        }
    }
    return phases.contains(today) ? today : phases.value(0);
}

QDate logDay(const QDateTime& now) {
    if (now.time() < QTime(6, 0)) {
        return now.date().addDays(-1);
    }
    return now.date();
}

static QString latestBulletFront(QString text) {
    static const QRegularExpression heading("^###\\s+(.*)$");
    static const QRegularExpression frontLine("^\\*\\*(.+?):?\\*\\*\\s*$");
    static const QRegularExpression bullet(
        "^[-*]\\s+\\\\?\\[\\s*(\\d{1,2}):(\\d{2})\\s*(AM|PM)\\s*\\\\?\\]\\s*-?"
        "\\s*(.*)$",
        QRegularExpression::CaseInsensitiveOption);
    text.remove(QRegularExpression(
        "<!--.*?-->", QRegularExpression::DotMatchesEverythingOption));
    bool inDone = false;
    QString front;
    QString latestFront;
    int latestMinute = -1;
    for (const QString& line : text.split('\n')) {
        QString trimmed = line.trimmed();
        QRegularExpressionMatch match = heading.match(trimmed);
        if (match.hasMatch()) {
            inDone = match.captured(1).toLower().contains("done today");
            front.clear();
            continue;
        }
        if (!inDone) {
            continue;
        }
        match = frontLine.match(trimmed);
        if (match.hasMatch()) {
            front = normFront(match.captured(1));
            continue;
        }
        match = bullet.match(trimmed);
        if (!match.hasMatch() || front.isEmpty() ||
            match.captured(4).trimmed().isEmpty()) {
            continue;
        }
        int hour = match.captured(1).toInt() % 12;
        if (match.captured(3).toUpper() == "PM") {
            hour += 12;
        }
        int minute =
            (hour * 60 + match.captured(2).toInt() + 18 * 60) % (24 * 60);
        if (minute >= latestMinute) {
            latestMinute = minute;
            latestFront = front;
        }
    }
    return latestFront;
}

QString chooseDefaultFront(const QStringList& fronts,
                           const QMap<QString, QString>& logs) {
    QStringList dates = logs.keys();
    for (auto date = dates.crbegin(); date != dates.crend(); ++date) {
        QString front = latestBulletFront(logs.value(*date));
        if (!front.isEmpty()) {
            return fronts.contains(front) ? front : fronts.value(0);
        }
    }
    return fronts.value(0);
}
