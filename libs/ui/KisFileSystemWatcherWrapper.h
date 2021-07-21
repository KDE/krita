#ifndef KISFILESYSTEMWATCHERWRAPPER_H
#define KISFILESYSTEMWATCHERWRAPPER_H

#include <QString>
#include <QHash>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QTextStream>
#include <QObject>

#include "kis_signal_compressor.h"

class KisFileSystemWatcherWrapper : public QObject
{
    Q_OBJECT

public:
    KisFileSystemWatcherWrapper();

    static QString unifyFilePath(const QString &path);
    bool addPath(const QString &file);
    bool removePath(const QString &file);

    QStringList files() const;

Q_SIGNALS:
    void fileChanged(const QString &path);

private Q_SLOTS:
    void slotFileChanged(const QString &path);
    void slotReattachLostFiles();

private:
    QFileSystemWatcher m_watcher;
    QHash<QString, int> m_pathCount;
    KisSignalCompressor m_reattachmentCompressor;
    QHash<QString, int> m_lostFilesAbsenceCounter;
};

#endif // KISFILESYSTEMWATCHERWRAPPER_H
