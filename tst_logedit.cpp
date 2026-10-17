#include "logedit.h"

#include <QTest>

static const QString bullet = R"(- \[ 10:00 AM \] - hi)";

static const QString dailyTemplate = R"(---
Front:
---
## **Tracks**

| Front | Phase | Load | Reason |
| :---- | :---- | :--: | :----- |
|       |       |      |        |

<!--
load scale
-->

---

### What's done today

**Front name:**
- task

<!--
loops: start a bullet with an emoji
- \[ 02:09 PM \] - ❓ does the scheduler fan out per target
-->

---
)";

class TestLogEdit : public QObject {
    Q_OBJECT

  private slots:
    void fronts_data();
    void fronts();
    void tracks_data();
    void tracks();
    void noTracksTable();
    void bullets_data();
    void bullets();
    void noDoneSection();
    void fullTemplate();
};

void TestLogEdit::fronts_data() {
    QTest::addColumn<QString>("log");
    QTest::addColumn<QString>("expected");

    QTest::newRow("empty list") << "---\nFront:\n---\nbody\n"
                                << "---\nFront:\n  - Quicklog\n---\nbody\n";
    QTest::newRow("added after the last item")
        << "---\nFront:\n  - A\n  - B\nStart: x\n---\n"
        << "---\nFront:\n  - A\n  - B\n  - Quicklog\nStart: x\n---\n";
    QTest::newRow("already listed") << "---\nFront:\n  - Quicklog\n---\n"
                                    << "---\nFront:\n  - Quicklog\n---\n";
    QTest::newRow("no Front key")
        << "---\nTags: x\n---\n"
        << "---\nTags: x\nFront:\n  - Quicklog\n---\n";
    QTest::newRow("no frontmatter")
        << "# notes\n"
        << "---\nFront:\n  - Quicklog\n---\n# notes\n";
}

void TestLogEdit::fronts() {
    QFETCH(QString, log);
    QFETCH(QString, expected);

    QCOMPARE(withFront(log, "Quicklog"), expected);
}

void TestLogEdit::tracks_data() {
    QTest::addColumn<QString>("log");
    QTest::addColumn<QString>("expected");

    QString header = "| Front | Phase | Load | Reason |\n"
                     "| :---- | :---- | :--: | :----- |\n";
    QTest::newRow("template row replaced")
        << header + "|       |       |      |        |\n\nafter\n"
        << header + "| Quicklog | m4 | | |\n\nafter\n";
    QTest::newRow("added after the last row")
        << header + "| Reading | ch6 | 1 | - |\n\nafter\n"
        << header +
               "| Reading | ch6 | 1 | - |\n| Quicklog | m4 | | |\n\nafter\n";
    QTest::newRow("front and phase already there")
        << header + "| Quicklog | m4 | N/A | AI |\n"
        << header + "| Quicklog | m4 | N/A | AI |\n";
    QTest::newRow("same front, other phase")
        << header + "| Quicklog | m3 | N/A | AI |\n"
        << header + "| Quicklog | m3 | N/A | AI |\n| Quicklog | m4 | | |\n";
    QTest::newRow("front cell cleaned before comparing")
        << header + "| **Quicklog:** | m4 | | |\n"
        << header + "| **Quicklog:** | m4 | | |\n";
}

void TestLogEdit::tracks() {
    QFETCH(QString, log);
    QFETCH(QString, expected);

    QCOMPARE(withTrack(log, "Quicklog", "m4").value_or("no table"), expected);
}

void TestLogEdit::noTracksTable() {
    QVERIFY(!withTrack("### What's done today\n", "Quicklog", "m4"));
}

void TestLogEdit::bullets_data() {
    QTest::addColumn<QString>("log");
    QTest::addColumn<QString>("expected");

    QTest::newRow("placeholder replaced") << R"(### What's done today

**Front name:**
- task

<!--
x
-->
)" << R"(### What's done today

**Quicklog:**
- \[ 10:00 AM \] - hi

<!--
x
-->
)";
    QTest::newRow("added at the end of the block") << R"(### What's done today

**Quicklog:**
- \[ 09:00 AM \] - first

**Reading:**
- \[ 09:30 AM \] - other
)" << R"(### What's done today

**Quicklog:**
- \[ 09:00 AM \] - first
- \[ 10:00 AM \] - hi

**Reading:**
- \[ 09:30 AM \] - other
)";
    QTest::newRow("empty bullet replaced") << R"(### What's done today

**Quicklog:**
- \[ 09:00 AM \] - first
-
)" << R"(### What's done today

**Quicklog:**
- \[ 09:00 AM \] - first
- \[ 10:00 AM \] - hi
)";
    QTest::newRow("empty timestamped bullet replaced")
        << R"(### What's done today

**Quicklog:**
- \[ 09:00 AM \] - first
- \[ 09:55 AM \] -
)" << R"(### What's done today

**Quicklog:**
- \[ 09:00 AM \] - first
- \[ 10:00 AM \] - hi
)";
    QTest::newRow("new block above the loops comment")
        << R"(### What's done today

**Reading:**
- \[ 09:30 AM \] - other

<!--
**Quicklog:**
- \[ 02:09 PM \] - ❓ example
-->

---
)" << R"(### What's done today

**Reading:**
- \[ 09:30 AM \] - other

**Quicklog:**
- \[ 10:00 AM \] - hi

<!--
**Quicklog:**
- \[ 02:09 PM \] - ❓ example
-->

---
)";
    QTest::newRow("older layout with more sections") << R"(### What's done today

**Reading:**
- \[ 09:30 AM \] - other
<!-- Factual record of session in bullets -->

---

### Snags

**Quicklog:**
- not here
)" << R"(### What's done today

**Reading:**
- \[ 09:30 AM \] - other

**Quicklog:**
- \[ 10:00 AM \] - hi

<!-- Factual record of session in bullets -->

---

### Snags

**Quicklog:**
- not here
)";
    QTest::newRow("no comment: new block above the ---")
        << R"(### What's done today

**Reading:**
- \[ 09:30 AM \] - other

---

### Snags
)" << R"(### What's done today

**Reading:**
- \[ 09:30 AM \] - other

**Quicklog:**
- \[ 10:00 AM \] - hi

---

### Snags
)";
    QTest::newRow("block header cleaned before comparing")
        << R"(### What's done today

**Quicklog**
- \[ 09:00 AM \] - first
)" << R"(### What's done today

**Quicklog**
- \[ 09:00 AM \] - first
- \[ 10:00 AM \] - hi
)";
}

void TestLogEdit::bullets() {
    QFETCH(QString, log);
    QFETCH(QString, expected);

    QCOMPARE(withBullet(log, "Quicklog", bullet).value_or("no section"),
             expected);
}

void TestLogEdit::noDoneSection() {
    QVERIFY(!withBullet("## **Tracks**\n", "Quicklog", bullet));
}

void TestLogEdit::fullTemplate() {
    QString log = withFront(dailyTemplate, "Quicklog");
    log = withTrack(log, "Quicklog", "milestone-4").value();
    log = withBullet(log, "Quicklog", bullet).value();

    QCOMPARE(log, QString(R"(---
Front:
  - Quicklog
---
## **Tracks**

| Front | Phase | Load | Reason |
| :---- | :---- | :--: | :----- |
| Quicklog | milestone-4 | | |

<!--
load scale
-->

---

### What's done today

**Quicklog:**
- \[ 10:00 AM \] - hi

<!--
loops: start a bullet with an emoji
- \[ 02:09 PM \] - ❓ does the scheduler fan out per target
-->

---
)"));
}

QTEST_GUILESS_MAIN(TestLogEdit)
#include "tst_logedit.moc"
