#include "fronts.h"

#include <QTest>

class TestFronts : public QObject {
    Q_OBJECT

  private slots:
    void newestFirst();
    void tiesKeepFirstAppearance();
    void frontsWithoutRowsGoLast();
    void rowsWithoutAProjectAreIgnored();
    void commentedRowsAreIgnored();
    void frontCellsAreCleaned_data();
    void frontCellsAreCleaned();
    void activeProjects_data();
    void activeProjects();
    void phaseTables_data();
    void phaseTables();
    void defaultPhases_data();
    void defaultPhases();
    void logDays_data();
    void logDays();
    void defaultFronts_data();
    void defaultFronts();
    void frontNames_data();
    void frontNames();
};

void TestFronts::newestFirst() {
    QMap<QString, QString> logs = {
        {"2026-01-01", "| Alpha | p | | |\n"},
        {"2026-01-02", "| Beta  | p | | |\n"},
    };
    QCOMPARE(orderFronts({"Alpha", "Beta"}, logs),
             QStringList({"Beta", "Alpha"}));
}

void TestFronts::tiesKeepFirstAppearance() {
    QMap<QString, QString> logs = {
        {"2026-01-01", "| Zeta  | p | | |\n"},
        {"2026-01-02", "| Alpha | p | | |\n| Zeta  | p | | |\n"},
    };
    QCOMPARE(orderFronts({"Alpha", "Zeta"}, logs),
             QStringList({"Zeta", "Alpha"}));
}

void TestFronts::frontsWithoutRowsGoLast() {
    QMap<QString, QString> logs = {{"2026-01-01", "| Mike | p | | |\n"}};
    QCOMPARE(orderFronts({"Zulu", "Mike", "bravo"}, logs),
             QStringList({"Mike", "bravo", "Zulu"}));
}

void TestFronts::rowsWithoutAProjectAreIgnored() {
    QMap<QString, QString> logs = {{"2026-01-01", "| Ghost | p | | |\n"}};
    QCOMPARE(orderFronts({"Alpha"}, logs), QStringList({"Alpha"}));
}

void TestFronts::commentedRowsAreIgnored() {
    QMap<QString, QString> logs = {
        {"2026-01-01", "| Beta  | p | | |\n"},
        {"2026-01-02", "| Front | Phase | Load | Reason |\n"
                       "| :---- | :---- | :--: | :----- |\n"
                       "| Alpha | p     |      |        |\n"
                       "<!--\n| Beta | p | | |\n-->\n"},
    };
    QCOMPARE(orderFronts({"Alpha", "Beta"}, logs),
             QStringList({"Alpha", "Beta"}));
}

void TestFronts::frontCellsAreCleaned_data() {
    QTest::addColumn<QString>("cell");
    QTest::addColumn<QString>("front");

    QTest::newRow("bold and colon") << "**Quicklog:**" << "Quicklog";
    QTest::newRow("alias") << "Interview prep" << "Interview-Prep";
    QTest::newRow("escaped link") << "Reading \\[x\\]" << "Reading";
}

void TestFronts::frontCellsAreCleaned() {
    QFETCH(QString, cell);
    QFETCH(QString, front);

    QMap<QString, QString> logs = {
        {"2026-01-01", "| " + cell + " | p | | |\n"}};
    QCOMPARE(orderFronts({"A", front}, logs), QStringList({front, "A"}));
}

void TestFronts::activeProjects_data() {
    QTest::addColumn<QString>("text");
    QTest::addColumn<bool>("active");

    QTest::newRow("in-progress") << "---\nStatus: in-progress\n---\n" << true;
    QTest::newRow("paused") << "Status: paused\n" << true;
    QTest::newRow("done") << "Status: done\n" << false;
    QTest::newRow("abandoned") << "Status: abandoned\n" << false;
    QTest::newRow("capitalised") << "Status: Done\n" << false;
    QTest::newRow("no status line") << "# Miscellaneous\n" << true;
}

void TestFronts::activeProjects() {
    QFETCH(QString, text);
    QFETCH(bool, active);

    QCOMPARE(isActiveProject(text), active);
}

void TestFronts::phaseTables_data() {
    QTest::addColumn<QString>("text");
    QTest::addColumn<QStringList>("phases");

    QTest::newRow("in-progress or empty status")
        << R"(| S.no. | **Phase** | Start | End | Status      | Approach |
| :---: | --------- | ----- | --- | ----------- | -------- |
|  1.   | *one*     |       |     | Done        |          |
|  2.   | *two*     |       |     | In-Progress |          |
|  3.   | *three*   |       |     |             |          |
|  4.   | *four*    |       |     | Paused      |          |
)" << QStringList({"two", "three"});

    QTest::newRow("any capitalisation")
        << R"(| S.no. | **Phase** | Start | End | Status      |
|  1.   | *one*     |       |     | in-progress |
|  2.   | *two*     |       |     | IN-PROGRESS |
)" << QStringList({"one", "two"});

    QTest::newRow("columns found by header")
        << R"(| Status | S.no. | **Phase** |
| Done   |  1.   | *one*     |
|        |  2.   | *two*     |
)" << QStringList({"two"});

    QTest::newRow("blank, plain and bold cells skipped")
        << R"(| S.no. | **Phase**  | Status |
|  1.   |            |        |
|  2.   | plain      |        |
|  3.   | **bold**   |        |
|  4.   | *a phrase* |        |
)" << QStringList({"a phrase"});

    QTest::newRow("stops at the end of the table")
        << R"(| S.no. | **Phase** | Status |
|  1.   | *one*     |        |

| 2. | *other* |
)" << QStringList({"one"});
}

void TestFronts::phaseTables() {
    QFETCH(QString, text);
    QFETCH(QStringList, phases);

    QCOMPARE(inProgressPhases(text), phases);
}

void TestFronts::defaultPhases_data() {
    QTest::addColumn<QStringList>("phases");
    QTest::addColumn<QString>("log");
    QTest::addColumn<QString>("expected");

    QStringList ab = {"a", "b"};
    QTest::newRow("today's row") << ab << "| F | b | | |\n" << "b";
    QTest::newRow("no log today") << ab << "" << "a";
    QTest::newRow("row for another front") << ab << "| G | b | | |\n" << "a";
    QTest::newRow("today's phase not in progress") << ab << "| F | old | | |\n"
                                                   << "a";
    QTest::newRow("last row wins")
        << QStringList({"b", "a"}) << "| F | b | | |\n| F | a | | |\n"
        << "a";
    QTest::newRow("front cell cleaned") << ab << "| **F:** | b | | |\n" << "b";
    QTest::newRow("commented row ignored") << ab << "<!--\n| F | b | | |\n-->\n"
                                           << "a";
    QTest::newRow("no phases") << QStringList() << "| F | b | | |\n" << "";
}

void TestFronts::defaultPhases() {
    QFETCH(QStringList, phases);
    QFETCH(QString, log);
    QFETCH(QString, expected);

    QCOMPARE(chooseDefaultPhase(phases, log, "F"), expected);
}

void TestFronts::logDays_data() {
    QTest::addColumn<QDateTime>("now");
    QTest::addColumn<QDate>("day");

    QDate oct16(2026, 10, 16);
    QDate oct17(2026, 10, 17);
    QTest::newRow("evening") << QDateTime(oct16, QTime(23, 59)) << oct16;
    QTest::newRow("midnight") << QDateTime(oct17, QTime(0, 0)) << oct16;
    QTest::newRow("4 AM") << QDateTime(oct17, QTime(4, 0)) << oct16;
    QTest::newRow("just before 6 AM")
        << QDateTime(oct17, QTime(5, 59, 59)) << oct16;
    QTest::newRow("6 AM") << QDateTime(oct17, QTime(6, 0)) << oct17;
    QTest::newRow("new month")
        << QDateTime(QDate(2026, 11, 1), QTime(3, 0)) << QDate(2026, 10, 31);
}

void TestFronts::logDays() {
    QFETCH(QDateTime, now);
    QFETCH(QDate, day);

    QCOMPARE(logDay(now), day);
}

void TestFronts::defaultFronts_data() {
    QTest::addColumn<QStringList>("fronts");
    QTest::addColumn<QString>("older");
    QTest::addColumn<QString>("newer");
    QTest::addColumn<QString>("expected");

    QStringList ab = {"A", "B"};
    const QString emptyBullet = "- \\[ 12:55 AM \\] - \n";
    QTest::newRow("latest time wins") << ab << "" << R"(### What's done today
**A:**
- \[ 10:00 AM \] - x
**B:**
- \[ 09:00 PM \] - y
)" << "B";
    QTest::newRow("after midnight is later")
        << ab << "" << R"(### What's done today
**B:**
- \[ 12:55 AM \] - y
**A:**
- \[ 11:41 PM \] - x
)" << "B";
    QTest::newRow("empty bullet ignored") << ab << ""
                                          << R"(### What's done today
**A:**
- \[ 11:41 PM \] - x
**B:**
)" + emptyBullet << "A";
    QTest::newRow("older log when today has none")
        << ab << R"(### What's done today
**B:**
- \[ 10:00 AM \] - y
)" << R"(### What's done today
**Front name:**
- task
)" << "B";
    QTest::newRow("newest log wins") << ab << R"(### What's done today
**A:**
- \[ 11:00 PM \] - x
)" << R"(### What's done today
**B:**
- \[ 09:00 AM \] - y
)" << "B";
    QTest::newRow("front not in the list")
        << ab << "" << R"(### What's done today
**Ghost:**
- \[ 10:00 AM \] - x
)" << "A";
    QTest::newRow("no bullets") << ab << "" << "" << "A";
    QTest::newRow("other sections ignored") << ab << "" << R"(### Plans
**A:**
- \[ 11:00 PM \] - x
### What's done today
**B:**
- \[ 09:00 AM \] - y
)" << "B";
    QTest::newRow("front name cleaned") << QStringList({"A", "Interview-Prep"})
                                        << "" << R"(### What's done today
**Interview prep:**
- \[ 10:00 AM \] - x
)" << "Interview-Prep";
    QTest::newRow("commented bullet ignored")
        << ab << "" << R"(### What's done today
**B:**
- \[ 09:00 AM \] - y
<!--
**A:**
- \[ 11:00 PM \] - x
-->
)" << "B";
}

void TestFronts::defaultFronts() {
    QFETCH(QStringList, fronts);
    QFETCH(QString, older);
    QFETCH(QString, newer);
    QFETCH(QString, expected);

    QMap<QString, QString> logs = {{"2026-01-01", older},
                                   {"2026-01-02", newer}};
    QCOMPARE(chooseDefaultFront(fronts, logs), expected);
}

void TestFronts::frontNames_data() {
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("valid");

    QTest::newRow("plain") << "Cogwars" << true;
    QTest::newRow("spaces and dashes") << "Interview Prep-2" << true;
    QTest::newRow("empty") << "" << false;
    QTest::newRow("slash") << "a/b" << false;
    QTest::newRow("backslash") << "a\\b" << false;
    QTest::newRow("colon") << "a:b" << false;
    QTest::newRow("link brackets") << "a[b]" << false;
    QTest::newRow("hash") << "a#b" << false;
    QTest::newRow("leading dot") << ".hidden" << false;
}

void TestFronts::frontNames() {
    QFETCH(QString, name);
    QFETCH(bool, valid);

    QCOMPARE(frontNameError(name).isEmpty(), valid);
}

QTEST_GUILESS_MAIN(TestFronts)
#include "tst_fronts.moc"
