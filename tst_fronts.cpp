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

    // "A" has no rows, so front only comes first if its cell was cleaned
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

QTEST_GUILESS_MAIN(TestFronts)
#include "tst_fronts.moc"
