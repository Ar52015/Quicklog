#include "backend.h"
#include "fronts.h"
#include "logedit.h"
#include "logfile.h"
#include "obsidian.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QTime>

static QString readText(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

QString Backend::vaultFolder() const {
    return QSettings().value("vaultFolder").toString();
}

void Backend::setVaultFolder(const QString& folder) {
    if (folder == vaultFolder()) {
        return;
    }
    QSettings().setValue("vaultFolder", folder);
    emit vaultFolderChanged();
    emit logExistsChanged();
    emit frontsChanged();
}

bool Backend::setVaultFolderUrl(const QUrl& folder) {
    QString vault = vaultFolderFor(folder.toLocalFile());
    if (vault.isEmpty()) {
        emit failed(QDir(folder.toLocalFile()).dirName() +
                    " isn't an Obsidian vault: pick the folder that holds "
                    ".obsidian.");
        return false;
    }
    setVaultFolder(vault);
    return true;
}

QString Backend::todayLogPath() const {
    QString name =
        logDay(QDateTime::currentDateTime()).toString("yyyy-MM-dd") + ".md";
    return QDir(vaultFolder()).filePath("Logs/" + name);
}

bool Backend::logExists() const {
    if (vaultFolder().isEmpty()) {
        return false;
    }
    return QFile::exists(todayLogPath());
}

QStringList Backend::fronts() const {
    if (vaultFolder().isEmpty()) {
        return {};
    }
    QDir projects(QDir(vaultFolder()).filePath("Projects"));
    QStringList names;
    for (const QFileInfo& file :
         projects.entryInfoList({"*.md"}, QDir::Files, QDir::Name)) {
        if (isActiveProject(readText(file.filePath()))) {
            names.append(file.completeBaseName());
        }
    }
    return orderFronts(names, readLogs());
}

QString Backend::defaultFront() const {
    return chooseDefaultFront(fronts(), readLogs());
}

QMap<QString, QString> Backend::readLogs() const {
    QDir logDir(QDir(vaultFolder()).filePath("Logs"));
    QMap<QString, QString> logs;
    for (const QFileInfo& file : logDir.entryInfoList({"*.md"}, QDir::Files)) {
        logs.insert(file.completeBaseName(), readText(file.filePath()));
    }
    return logs;
}

QString Backend::currentFront() const { return m_currentFront; }

void Backend::setCurrentFront(const QString& front) {
    if (front == m_currentFront) {
        return;
    }
    m_currentFront = front;
    emit currentFrontChanged();
    emit phasesChanged();
}

QStringList Backend::phases() const {
    if (vaultFolder().isEmpty() || m_currentFront.isEmpty()) {
        return {};
    }
    QString text = readText(projectPath(m_currentFront));
    return inProgressPhases(text) + ::pausedPhases(text);
}

QStringList Backend::pausedPhases() const {
    if (vaultFolder().isEmpty() || m_currentFront.isEmpty()) {
        return {};
    }
    return ::pausedPhases(readText(projectPath(m_currentFront)));
}

QString Backend::defaultPhase() const {
    if (vaultFolder().isEmpty() || m_currentFront.isEmpty()) {
        return {};
    }
    return chooseDefaultPhase(
        inProgressPhases(readText(projectPath(m_currentFront))),
        readText(todayLogPath()), m_currentFront);
}

QString Backend::projectPath(const QString& front) const {
    return QDir(vaultFolder()).filePath("Projects/" + front + ".md");
}

void Backend::refresh() {
    emit logExistsChanged();
    emit frontsChanged();
    emit phasesChanged();
}

void Backend::askForLog() {
    QDateTime now = QDateTime::currentDateTime();
    QDate day = logDay(now);
    if (day != now.date()) {
        emit failed("It's before 6 AM: create " + day.toString("yyyy-MM-dd") +
                    ".md in Obsidian, then press ↻.");
        return;
    }
    if (!QDesktopServices::openUrl(
            obsidianDailyUrl(QDir(vaultFolder()).dirName()))) {
        emit failed("Couldn't open Obsidian.");
        return;
    }
    emit notice("Asked Obsidian for today's log. Press ↻ once it's there.");
}

void Backend::addFront(const QString& name) {
    QString front = name.trimmed();
    QString error = frontNameError(front);
    if (!error.isEmpty()) {
        emit failed(error);
        return;
    }
    QDir vault(vaultFolder());
    QString file = "Projects/" + front + ".md";
    if (QFile::exists(vault.filePath(file))) {
        emit failed(file + " already exists.");
        return;
    }
    if (!QFile::copy(vault.filePath("templates/project.md"),
                     vault.filePath(file))) {
        emit failed("Couldn't create " + file + " from templates/project.md.");
        return;
    }
    emit frontsChanged();
    emit frontAdded(front);
    openFront(front);
}

void Backend::openFront(const QString& front) {
    QString path = QDir(vaultFolder()).filePath("Projects/" + front + ".md");
    if (!QDesktopServices::openUrl(obsidianOpenUrl(path))) {
        emit failed("Couldn't open Obsidian.");
    }
}

void Backend::addPhase(const QString& front, const QString& name,
                       const QString& approach) {
    if (front.isEmpty()) {
        emit failed("There's no front to add a phase to.");
        return;
    }
    QString phase = name.trimmed();
    QString error = phaseNameError(phase);
    if (!error.isEmpty()) {
        emit failed(error);
        return;
    }
    if (std::optional<QString> failure =
            addPhaseToProject(projectPath(front), phase, approach.trimmed(),
                              logDay(QDateTime::currentDateTime()))) {
        emit failed(*failure);
        return;
    }
    emit phasesChanged();
    emit phaseAdded(phase);
}

QString Backend::phaseApproach(const QString& front,
                               const QString& phase) const {
    return ::phaseApproach(readText(projectPath(front)), phase);
}

QString Backend::trackLabel(const QString& front, const QString& phase) const {
    if (front.isEmpty() || phase.isEmpty()) {
        return {};
    }
    QString track = trackNameFor(phase, allPhases(readText(projectPath(front))),
                                 readLogs(), front);
    if (track == phase) {
        return {};
    }
    if (track.startsWith(phase + "-", Qt::CaseInsensitive)) {
        return track.mid(phase.size() + 1);
    }
    return track;
}

void Backend::startSubphase(const QString& front, const QString& phase,
                            const QString& part) {
    if (front.isEmpty() || phase.isEmpty()) {
        emit failed("Pick a phase first.");
        return;
    }
    const QString name = part.trimmed();
    const QString error = phaseNameError(name);
    if (!error.isEmpty()) {
        emit failed(error);
        return;
    }
    if (!logExists()) {
        askForLog();
        return;
    }
    if (std::optional<QString> failure = addToLog(
            todayLogPath(), front, subphaseName(phase, name), QString())) {
        emit failed(*failure);
        return;
    }
    emit phasesChanged();
    emit phaseAdded(phase);
}

void Backend::setPhaseStatus(const QString& front, const QString& phase,
                             const QString& status, const QString& approach) {
    if (front.isEmpty() || phase.isEmpty()) {
        emit failed("Pick a phase first.");
        return;
    }
    if (std::optional<QString> failure = setPhaseStatusInProject(
            projectPath(front), phase, status, approach.trimmed(),
            logDay(QDateTime::currentDateTime()))) {
        emit failed(*failure);
        return;
    }
    emit phasesChanged();
    emit phaseStatusChanged(phase);
}

void Backend::addEntry(const QString& front, const QString& phase,
                       const QString& type, const QString& text) {
    if (!logExists()) {
        askForLog();
        return;
    }
    if (front.isEmpty()) {
        emit failed("There's no front to add this to.");
        return;
    }
    QString bullet = text.trimmed().isEmpty()
                         ? QString()
                         : formatBullet(type, text.trimmed());
    const QString track =
        phase.isEmpty()
            ? QString()
            : trackNameFor(phase, allPhases(readText(projectPath(front))),
                           readLogs(), front);
    if (std::optional<QString> error =
            addToLog(todayLogPath(), front, track, bullet)) {
        emit failed(*error);
        return;
    }
    emit entryAdded();
}

QString Backend::formatBullet(const QString& type, const QString& text) const {
    return bulletLine(QTime::currentTime(), type, text);
}

QString Backend::previewBullet(const QString& type, const QString& text) const {
    return wikilinksAsLinks(formatBullet(type, text));
}
