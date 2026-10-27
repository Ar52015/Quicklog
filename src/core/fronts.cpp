#include "fronts.h"

#include <QHash>
#include <QRegularExpression>
#include <algorithm>

QString normFront(QString cell) {
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

QStringList tableCells(const QString& line) {
    QString row = line.trimmed();
    if (!row.startsWith('|')) {
        return {};
    }
    if (row.endsWith('|') && !row.endsWith("\\|")) {
        row.chop(1);
    }
    static const QRegularExpression border(R"((?<!\\)\|)");
    QStringList cells = row.mid(1).split(border);
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

static QStringList phasesWhere(const QString& projectText,
                               bool (*accept)(const QString& status)) {
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
        if (match.hasMatch() && accept(status)) {
            phases.append(match.captured(1).trimmed());
        }
    }
    return phases;
}

QStringList inProgressPhases(const QString& projectText) {
    return phasesWhere(projectText, [](const QString& status) {
        return status.isEmpty() || status == "in-progress";
    });
}

QStringList allPhases(const QString& projectText) {
    return phasesWhere(projectText, [](const QString&) { return true; });
}

QStringList pausedPhases(const QString& projectText) {
    return phasesWhere(
        projectText, [](const QString& status) { return status == "paused"; });
}

static QString nameSlug(const QString& name) {
    static const QRegularExpression other("[^a-z0-9]+");
    static const QRegularExpression edges("^-+|-+$");
    return name.toLower().replace(other, "-").remove(edges);
}

QString phaseOwner(const QString& trackName, const QStringList& phases) {
    const QString name = nameSlug(trackName);
    if (name.isEmpty()) {
        return {};
    }
    QString owner;
    for (const QString& phase : phases) {
        const QString slug = nameSlug(phase);
        if (slug == name) {
            return phase;
        }
        if (!slug.isEmpty() && name.startsWith(slug + "-") &&
            slug.size() > nameSlug(owner).size()) {
            owner = phase;
        }
    }
    if (!owner.isEmpty()) {
        return owner;
    }
    QString shortened;
    int matches = 0;
    for (const QString& phase : phases) {
        if (nameSlug(phase).startsWith(name + "-")) {
            shortened = phase;
            ++matches;
        }
    }
    return matches == 1 ? shortened : QString();
}

QString trackNameFor(const QString& phase, const QStringList& phases,
                     const QMap<QString, QString>& logs, const QString& front) {
    for (auto log = logs.constEnd(); log != logs.constBegin();) {
        --log;
        const QList<QStringList> rows = trackRows(log.value());
        for (auto row = rows.crbegin(); row != rows.crend(); ++row) {
            if (normFront(row->at(0)) == front &&
                phaseOwner(row->at(1), phases) == phase) {
                return row->at(1);
            }
        }
    }
    return phase;
}

QString subphaseName(const QString& phase, const QString& part) {
    static const QRegularExpression spaces("\\s+");
    return phase + "-" + part.trimmed().replace(spaces, "-");
}

QString chooseDefaultPhase(const QStringList& phases, const QString& todaysLog,
                           const QString& front) {
    QString today;
    for (const QStringList& cells : trackRows(todaysLog)) {
        if (normFront(cells[0]) == front) {
            QString owner = phaseOwner(cells[1], phases);
            if (!owner.isEmpty()) {
                today = owner;
            }
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

QString frontNameError(const QString& name) {
    static const QString forbidden = "\\/:*?\"<>|#^[]";
    if (name.isEmpty()) {
        return "Type a name for the new front.";
    }
    if (name.startsWith('.')) {
        return "A front name can't start with a dot.";
    }
    for (QChar c : forbidden) {
        if (name.contains(c)) {
            return "A front name can't contain any of \\ / : * ? \" < > | # ^ "
                   "[ ].";
        }
    }
    return {};
}

QString phaseNameError(const QString& name) {
    if (name.isEmpty()) {
        return "Type a name for the new phase.";
    }
    if (name.contains('|') || name.contains('*')) {
        return "A phase name can't contain | or *.";
    }
    return {};
}

static qsizetype phaseTableHeader(const QStringList& lines,
                                  QStringList& columns) {
    for (qsizetype i = 0; i < lines.size(); ++i) {
        columns = tableCells(lines[i]);
        if (columns.contains("**Phase**")) {
            return i;
        }
    }
    return -1;
}

static QString phaseName(const QString& cell) {
    static const QRegularExpression italic("^\\*([^*]+)\\*$");
    QRegularExpressionMatch match = italic.match(cell);
    return match.hasMatch() ? match.captured(1).trimmed() : QString();
}

static qsizetype phaseRow(const QStringList& lines, qsizetype header,
                          qsizetype phaseColumn, const QString& name) {
    for (qsizetype i = header + 1; i < lines.size(); ++i) {
        QStringList cells = tableCells(lines[i]);
        if (cells.isEmpty()) {
            break;
        }
        QString phase = phaseName(cells.value(phaseColumn));
        if (!phase.isEmpty() && phase.compare(name, Qt::CaseInsensitive) == 0) {
            return i;
        }
    }
    return -1;
}

static QString tableRow(const QStringList& cells) {
    QString row = "|";
    for (const QString& cell : cells) {
        row += cell.isEmpty() ? QString(" |") : " " + cell + " |";
    }
    return row;
}

static QString escapedCell(QString text) { return text.replace('|', "\\|"); }

std::optional<QString> withPhase(const QString& projectText,
                                 const QString& name, const QString& approach,
                                 const QDate& day) {
    static const QRegularExpression separator("^:?-{2,}");
    static const QRegularExpression number("^(\\d+)\\.?$");
    QStringList lines = projectText.split('\n');
    QStringList columns;
    const qsizetype header = phaseTableHeader(lines, columns);
    if (header < 0) {
        return std::nullopt;
    }
    const qsizetype phaseColumn = columns.indexOf("**Phase**");
    if (phaseRow(lines, header, phaseColumn, name) >= 0) {
        return projectText;
    }
    const qsizetype numberColumn = columns.indexOf("S.no.");

    qsizetype last = header;
    qsizetype emptyRow = -1;
    QString emptyRowNumber;
    int highest = 0;
    for (qsizetype i = header + 1; i < lines.size(); ++i) {
        QStringList cells = tableCells(lines[i]);
        if (cells.isEmpty()) {
            break;
        }
        last = i;
        QString phaseCell = cells.value(phaseColumn);
        if (phaseCell.contains(separator)) {
            continue;
        }
        QRegularExpressionMatch numbered =
            number.match(cells.value(numberColumn));
        if (numbered.hasMatch()) {
            highest = qMax(highest, numbered.captured(1).toInt());
        }
        if (phaseCell.isEmpty() && emptyRow < 0) {
            emptyRow = i;
            emptyRowNumber = cells.value(numberColumn);
        }
    }

    QString rowNumber = emptyRowNumber.isEmpty()
                            ? QString::number(highest + 1) + "."
                            : emptyRowNumber;
    QStringList cells;
    for (const QString& column : columns) {
        if (column == "S.no.") {
            cells.append(rowNumber);
        } else if (column == "**Phase**") {
            cells.append("*" + name + "*");
        } else if (column == "Start") {
            cells.append("[[" + day.toString("yyyy-MM-dd") + "]]");
        } else if (column == "Status") {
            cells.append("In-Progress");
        } else if (column == "Approach") {
            cells.append(escapedCell(approach));
        } else {
            cells.append(QString());
        }
    }
    if (emptyRow >= 0) {
        lines[emptyRow] = tableRow(cells);
    } else {
        lines.insert(last + 1, tableRow(cells));
    }
    return lines.join('\n');
}

QString phaseApproach(const QString& projectText, const QString& name) {
    QStringList lines = projectText.split('\n');
    QStringList columns;
    const qsizetype header = phaseTableHeader(lines, columns);
    if (header < 0) {
        return {};
    }
    const qsizetype row =
        phaseRow(lines, header, columns.indexOf("**Phase**"), name);
    if (row < 0) {
        return {};
    }
    QString approach =
        tableCells(lines[row]).value(columns.indexOf("Approach"));
    return approach.replace("\\|", "|");
}

std::optional<QString> withPhaseStatus(const QString& projectText,
                                       const QString& name,
                                       const QString& status,
                                       const QString& approach,
                                       const QDate& day) {
    QStringList lines = projectText.split('\n');
    QStringList columns;
    const qsizetype header = phaseTableHeader(lines, columns);
    if (header < 0) {
        return std::nullopt;
    }
    const qsizetype row =
        phaseRow(lines, header, columns.indexOf("**Phase**"), name);
    if (row < 0) {
        return std::nullopt;
    }
    QStringList cells = tableCells(lines[row]);
    while (cells.size() < columns.size()) {
        cells.append(QString());
    }
    for (qsizetype c = 0; c < columns.size(); ++c) {
        if (columns[c] == "Status") {
            cells[c] = status;
        } else if (columns[c] == "End" &&
                   (status == "Done" || status == "Abandoned")) {
            cells[c] = "[[" + day.toString("yyyy-MM-dd") + "]]";
        } else if (columns[c] == "Approach") {
            cells[c] = escapedCell(approach);
        }
    }
    lines[row] = tableRow(cells);
    return lines.join('\n');
}
