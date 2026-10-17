#include "obsidian.h"

#include <QTest>

class TestObsidian : public QObject {
    Q_OBJECT

  private slots:
    void openUrl();
};

void TestObsidian::openUrl() {
    QUrl url = obsidianOpenUrl("/home/x/my vault/Projects/New front.md");
    QCOMPARE(url.toString(QUrl::FullyEncoded),
             "obsidian://open?path="
             "%2Fhome%2Fx%2Fmy%20vault%2FProjects%2FNew%20front");
}

QTEST_GUILESS_MAIN(TestObsidian)
#include "tst_obsidian.moc"
