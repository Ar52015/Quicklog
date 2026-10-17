#include "logedit.h"
#include "fronts.h"

#include <QRegularExpression>
#include <QStringList>

QString withFront(const QString& log, const QString& front) {
    QString item = "  - " + front;
    QStringList lines = log.split('\n');
    qsizetype close = -1;
    if (lines.value(0).trimmed() == "---") {
        for (qsizetype i = 1; i < lines.size(); ++i) {
            if (lines[i].trimmed() == "---") {
                close = i;
                break;
            }
        }
    }
    if (close < 0) {
        return "---\nFront:\n" + item + "\n---\n" + log;
    }

    qsizetype key = -1;
    for (qsizetype i = 1; i < close; ++i) {
        if (lines[i].startsWith("Front:")) {
            key = i;
            break;
        }
    }
    if (key < 0) {
        lines.insert(close, item);
        lines.insert(close, "Front:");
        return lines.join('\n');
    }

    static const QRegularExpression listItem("^\\s+-\\s+(.*)$");
    qsizetype last = key;
    for (qsizetype i = key + 1; i < close; ++i) {
        QRegularExpressionMatch match = listItem.match(lines[i]);
        if (!match.hasMatch()) {
            break;
        }
        if (match.captured(1).trimmed() == front) {
            return log;
        }
        last = i;
    }
    lines.insert(last + 1, item);
    return lines.join('\n');
}

std::optional<QString> withTrack(const QString& log, const QString& front,
                                 const QString& phase) {
    static const QRegularExpression separator("^:?-{2,}");
    QStringList lines = log.split('\n');
    qsizetype header = -1;
    for (qsizetype i = 0; i < lines.size(); ++i) {
        QStringList cells = tableCells(lines[i]);
        if (cells.size() >= 2 && cells[0].toLower() == "front") {
            header = i;
            break;
        }
    }
    if (header < 0) {
        return std::nullopt;
    }

    qsizetype last = header;
    qsizetype emptyRow = -1;
    for (qsizetype i = header + 1; i < lines.size(); ++i) {
        QStringList cells = tableCells(lines[i]);
        if (cells.isEmpty()) {
            break;
        }
        last = i;
        if (cells[0].contains(separator)) {
            continue;
        }
        if (cells.join("").isEmpty()) {
            if (emptyRow < 0) {
                emptyRow = i;
            }
            continue;
        }
        if (normFront(cells[0]) == front && cells.value(1) == phase) {
            return log;
        }
    }

    qsizetype columns = tableCells(lines[header]).size();
    QString row = "| " + front + " | " + phase + " |" +
                  QString(" |").repeated(qMax<qsizetype>(columns - 2, 0));
    if (emptyRow >= 0) {
        lines[emptyRow] = row;
    } else {
        lines.insert(last + 1, row);
    }
    return lines.join('\n');
}

static bool isEmptyBullet(const QString& line) {
    static const QRegularExpression empty(
        "^[-*](\\s+\\\\?\\[\\s*\\d{1,2}:\\d{2}\\s*(AM|PM)\\s*\\\\?\\]\\s*-?)?"
        "\\s*"
        "$",
        QRegularExpression::CaseInsensitiveOption);
    return empty.match(line.trimmed()).hasMatch();
}

std::optional<QString> withBullet(const QString& log, const QString& front,
                                  const QString& bullet) {
    static const QRegularExpression heading("^#{1,6}\\s+(.*)$");
    static const QRegularExpression frontLine("^\\*\\*(.+?):?\\*\\*\\s*$");
    QStringList lines = log.split('\n');

    qsizetype done = -1;
    for (qsizetype i = 0; i < lines.size(); ++i) {
        QRegularExpressionMatch match = heading.match(lines[i].trimmed());
        if (match.hasMatch() &&
            match.captured(1).toLower().contains("done today")) {
            done = i;
            break;
        }
    }
    if (done < 0) {
        return std::nullopt;
    }

    qsizetype end = lines.size();
    for (qsizetype i = done + 1; i < lines.size(); ++i) {
        QString line = lines[i].trimmed();
        if (heading.match(line).hasMatch() || line == "---") {
            end = i;
            break;
        }
    }

    qsizetype firstComment = -1;
    qsizetype placeholder = -1;
    qsizetype block = -1;
    bool inComment = false;
    for (qsizetype i = done + 1; i < end; ++i) {
        QString line = lines[i].trimmed();
        if (inComment) {
            inComment = !line.contains("-->");
            continue;
        }
        if (line.startsWith("<!--")) {
            if (firstComment < 0) {
                firstComment = i;
            }
            inComment = !line.contains("-->");
            continue;
        }
        QRegularExpressionMatch match = frontLine.match(line);
        if (!match.hasMatch()) {
            continue;
        }
        if (line == "**Front name:**" &&
            lines.value(i + 1).trimmed() == "- task") {
            placeholder = i;
        } else if (block < 0 && normFront(match.captured(1)) == front) {
            block = i;
        }
    }

    if (block >= 0) {
        qsizetype last = block;
        while (last + 1 < end) {
            QString line = lines[last + 1].trimmed();
            if (line.isEmpty() || line.startsWith("<!--") ||
                frontLine.match(line).hasMatch()) {
                break;
            }
            ++last;
        }
        if (last > block && isEmptyBullet(lines[last])) {
            lines[last] = bullet;
        } else {
            lines.insert(last + 1, bullet);
        }
        return lines.join('\n');
    }

    if (placeholder >= 0) {
        lines[placeholder] = "**" + front + ":**";
        lines[placeholder + 1] = bullet;
        return lines.join('\n');
    }

    qsizetype at = firstComment >= 0 ? firstComment : end;
    QStringList newBlock = {"**" + front + ":**", bullet, ""};
    if (at > 0 && !lines[at - 1].trimmed().isEmpty()) {
        newBlock.prepend("");
    }
    for (qsizetype i = newBlock.size() - 1; i >= 0; --i) {
        lines.insert(at, newBlock[i]);
    }
    return lines.join('\n');
}
