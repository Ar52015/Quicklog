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

QTEST_GUILESS_MAIN(TestObsidian)
#include "tst_obsidian.moc"
