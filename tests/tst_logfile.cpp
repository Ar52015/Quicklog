#include "logfile.h"

#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>
#include <memory>

static const QString bullet = R"(- \[ 10:00 AM \] - hi)";

static QByteArray readBytes(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return file.readAll();
}

static bool writeBytes(const QString& path, const QByteArray& bytes) {
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
}

class TestLogFile : public QObject {
    Q_OBJECT

  private slots:
    void init();
    void writesTheEntry();
    void addsFrontWithoutLogs();
    void refusesWhileTemplaterRuns();
    void keepsCrlfLineEndings();
    void writesUtf8();
    void failedWriteLeavesFileUnchanged();
    void missingLogIsAnError();
    void addsThePhase();
    void phaseKeepsCrlf();
    void existingPhaseIsAnError();
    void projectWithoutTableIsAnError();

  private:
    std::unique_ptr<QTemporaryDir> dir;
    QString log;
    QString project;
};

void TestLogFile::init() {
    dir = std::make_unique<QTemporaryDir>();
    QVERIFY(dir->isValid());
    log = dir->filePath("today.md");
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/today.md"), log));
    project = dir->filePath("Quicklog.md");
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/project.md"), project));
}

void TestLogFile::writesTheEntry() {
    std::optional<QString> error =
        addToLog(log, "Quicklog", "milestone-4", bullet);
    QVERIFY2(!error, qPrintable(error.value_or("")));
    QCOMPARE(readBytes(log),
             readBytes(QFINDTESTDATA("testdata/today-quicklog.md")));
}

void TestLogFile::addsFrontWithoutLogs() {
    std::optional<QString> error = addToLog(log, "Quicklog", "milestone-4", "");
    QVERIFY2(!error, qPrintable(error.value_or("")));
    QByteArray bytes = readBytes(log);
    QVERIFY(bytes.contains("Front:\n  - Quicklog\n---"));
    QVERIFY(bytes.contains("| Quicklog | milestone-4 | | |\n"));
    QVERIFY(!bytes.contains("**Quicklog:**"));
    QVERIFY(bytes.contains("**Front name:**\n- task\n"));
}

void TestLogFile::refusesWhileTemplaterRuns() {
    QVERIFY(writeBytes(log, readBytes(log) + "<%* tp.user.tracker(tp) %>\n"));
    QByteArray before = readBytes(log);
    QVERIFY(addToLog(log, "Quicklog", "milestone-4", bullet));
    QCOMPARE(readBytes(log), before);
}

void TestLogFile::keepsCrlfLineEndings() {
    QVERIFY(writeBytes(log, readBytes(log).replace("\n", "\r\n")));
    std::optional<QString> error =
        addToLog(log, "Quicklog", "milestone-4", bullet);
    QVERIFY2(!error, qPrintable(error.value_or("")));
    QByteArray bytes = readBytes(log);
    QVERIFY(bytes.contains("**Quicklog:**\r\n"));
    QCOMPARE(bytes.count("\r\n"), bytes.count("\n"));
}

void TestLogFile::writesUtf8() {
    QString text = R"(- \[ 10:00 AM \] - café — 日本語 ✓)";
    std::optional<QString> error = addToLog(log, "Quicklog", "m", text);
    QVERIFY2(!error, qPrintable(error.value_or("")));
    QVERIFY(readBytes(log).contains(text.toUtf8()));
}

void TestLogFile::failedWriteLeavesFileUnchanged() {
    QByteArray before = readBytes(log);
    QFile::setPermissions(dir->path(), QFile::ReadOwner | QFile::ExeOwner);
    if (QFileInfo(dir->path()).isWritable()) {
        QFile::setPermissions(dir->path(), QFile::ReadOwner |
                                               QFile::WriteOwner |
                                               QFile::ExeOwner);
        QSKIP("The folder is still writable, as it is for root.");
    }
    std::optional<QString> error =
        addToLog(log, "Quicklog", "milestone-4", bullet);
    QFile::setPermissions(dir->path(), QFile::ReadOwner | QFile::WriteOwner |
                                           QFile::ExeOwner);
    QVERIFY(error);
    QVERIFY2(error->startsWith("Couldn't write"), qPrintable(*error));
    QCOMPARE(readBytes(log), before);
}

void TestLogFile::missingLogIsAnError() {
    std::optional<QString> error =
        addToLog(dir->filePath("missing.md"), "Quicklog", "m", bullet);
    QVERIFY(error);
    QVERIFY2(error->startsWith("Couldn't read"), qPrintable(*error));
}

void TestLogFile::addsThePhase() {
    std::optional<QString> error =
        addPhaseToProject(project, "ship", QDate(2026, 10, 18));
    QVERIFY2(!error, qPrintable(error.value_or("")));
    QCOMPARE(readBytes(project),
             readBytes(QFINDTESTDATA("testdata/project-ship.md")));
}

void TestLogFile::phaseKeepsCrlf() {
    QByteArray crlf = readBytes(project).replace("\n", "\r\n");
    QVERIFY(writeBytes(project, crlf));
    std::optional<QString> error =
        addPhaseToProject(project, "ship", QDate(2026, 10, 18));
    QVERIFY2(!error, qPrintable(error.value_or("")));
    QByteArray bytes = readBytes(project);
    QVERIFY(bytes.contains(
        "| 3. | *ship* | [[2026-10-18]] | | In-Progress | |\r\n"));
    QVERIFY(!bytes.replace("\r\n", "").contains('\n'));
}

void TestLogFile::existingPhaseIsAnError() {
    QByteArray before = readBytes(project);
    std::optional<QString> error =
        addPhaseToProject(project, "Setup", QDate(2026, 10, 18));
    QCOMPARE(error.value_or(""), "Setup is already a phase in Quicklog.md.");
    QCOMPARE(readBytes(project), before);
}

void TestLogFile::projectWithoutTableIsAnError() {
    QVERIFY(writeBytes(project, "# Notes\n\nNo table here.\n"));
    std::optional<QString> error =
        addPhaseToProject(project, "ship", QDate(2026, 10, 18));
    QCOMPARE(error.value_or(""), "Quicklog.md has no phase table.");
    QVERIFY2(!addPhaseToProject(dir->filePath("Missing.md"), "ship",
                                QDate(2026, 10, 18))
                  .value_or("")
                  .isEmpty(),
             "a missing project file should be an error");
}

QTEST_GUILESS_MAIN(TestLogFile)
#include "tst_logfile.moc"
