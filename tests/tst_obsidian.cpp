#include "obsidian.h"

#include <QDir>
#include <QTemporaryDir>
#include <QTest>

class TestObsidian : public QObject {
    Q_OBJECT

  private slots:
    void openUrl();
    void dailyUrl();
    void vaultFolder();
    void wikilinks_data();
    void wikilinks();
};

void TestObsidian::openUrl() {
    QUrl url = obsidianOpenUrl("/home/x/my vault/Projects/New front.md");
    QCOMPARE(url.toString(QUrl::FullyEncoded),
             "obsidian://open?path="
             "%2Fhome%2Fx%2Fmy%20vault%2FProjects%2FNew%20front");
}

void TestObsidian::dailyUrl() {
    QCOMPARE(obsidianDailyUrl("qa-vault").toString(QUrl::FullyEncoded),
             "obsidian://daily?vault=qa-vault");
    QCOMPARE(obsidianDailyUrl("my vault").toString(QUrl::FullyEncoded),
             "obsidian://daily?vault=my%20vault");
}

void TestObsidian::vaultFolder() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir root(dir.path());
    QVERIFY(root.mkpath("vault/.obsidian"));
    QVERIFY(root.mkpath("vault/Logs/old"));
    QVERIFY(root.mkpath("plain"));
    const QString vault = root.filePath("vault");

    QCOMPARE(vaultFolderFor(vault), vault);
    QCOMPARE(vaultFolderFor(root.filePath("vault/Logs")), vault);
    QCOMPARE(vaultFolderFor(root.filePath("vault/Logs/old")), QString());
    QCOMPARE(vaultFolderFor(root.filePath("plain")), QString());
}

void TestObsidian::wikilinks_data() {
    QTest::addColumn<QString>("markdown");
    QTest::addColumn<QString>("expected");

    QTest::newRow("plain") << "see [[some file]]" << "see [some file]()";
    QTest::newRow("alias") << "[[Projects/Quicklog|ql]]" << "[ql]()";
    QTest::newRow("two") << "[[a]] and [[b|c]]" << "[a]() and [c]()";
    QTest::newRow("embed stays") << "![[shot.png]]" << "![[shot.png]]";
    QTest::newRow("markdown link stays") << "[x](y)" << "[x](y)";
}

void TestObsidian::wikilinks() {
    QFETCH(QString, markdown);
    QFETCH(QString, expected);
    QCOMPARE(wikilinksAsLinks(markdown), expected);
}

QTEST_GUILESS_MAIN(TestObsidian)
#include "tst_obsidian.moc"
