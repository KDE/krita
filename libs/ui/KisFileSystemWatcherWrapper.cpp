#include "KisFileSystemWatcherWrapper.h"
#include "kis_signal_compressor.h"
#include "kis_thread_safe_signal_compressor.h"
#include "KisUsageLogger.h"
#include <KoStore.h>
#include <kis_paint_layer.h>


KisFileSystemWatcherWrapper::KisFileSystemWatcherWrapper()
    : m_reattachmentCompressor(100, KisSignalCompressor::FIRST_INACTIVE)
{
    connect(&m_watcher, SIGNAL(fileChanged(QString)), SLOT(slotFileChanged(QString)));
    connect(&m_reattachmentCompressor, SIGNAL(timeout()), SLOT(slotReattachLostFiles()));
}   

bool KisFileSystemWatcherWrapper::addPath(const QString &file)
{
    bool result = true;
    const QString ufile = unifyFilePath(file);

    if (m_pathCount.contains(ufile)) {
        m_pathCount[ufile]++;
        }
    else {
        m_pathCount.insert(ufile, 1);
        result = m_watcher.addPath(ufile);
        }

        return result;
    }

bool KisFileSystemWatcherWrapper::removePath(const QString &file)
{
    bool result = true;
    const QString ufile = unifyFilePath(file);

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_pathCount.contains(ufile), false);

    if (m_pathCount[ufile] == 1) {
        m_pathCount.remove(ufile);
        result = m_watcher.removePath(ufile);
       }
    else {
        m_pathCount[ufile]--;
       }
    return result;
}

QStringList KisFileSystemWatcherWrapper::files() const
{
    return m_watcher.files();
}

void KisFileSystemWatcherWrapper::slotFileChanged(const QString &path)
{
    // re-add the file after QSaveFile optimization
    if (!m_watcher.files().contains(path)) {

        if (QFileInfo(path).exists()) {
            m_watcher.addPath(path);
            m_lostFilesAbsenceCounter.remove(path);
            emit fileChanged(path);
          }
        else {
            if (m_lostFilesAbsenceCounter.contains(path)) {
                m_lostFilesAbsenceCounter[path]++;
                }
            else {
                m_lostFilesAbsenceCounter[path] = 0;
                }

            const int absenceTimeMSec =
            m_reattachmentCompressor.delay() * m_lostFilesAbsenceCounter[path];

            const bool shouldSpitWarning =
                absenceTimeMSec <= 600000 &&
                    ((absenceTimeMSec >= 60000 && (absenceTimeMSec % 60000 == 0)) ||
                     (absenceTimeMSec >= 10000 && (absenceTimeMSec % 10000 == 0)));

            if (shouldSpitWarning) {
                QString message;
                QTextStream log(&message);

                log << "WARNING: couldn't reconnect to a removed file layer's file (" << path << "). File is not available for " << absenceTimeMSec / 1000 << " seconds";

                qWarning() << message;
                KisUsageLogger::log(message);

                if (absenceTimeMSec == 600000) {
                    message.clear();
                    log.reset();

                    log << "Giving up... :( No more reports about " << path;

                    qWarning() << message;
                    KisUsageLogger::log(message);
                    }
                }

                m_reattachmentCompressor.start();
            }
        }
    else {
        emit fileChanged(path);
       }
}

void KisFileSystemWatcherWrapper::slotReattachLostFiles()
{
    const QList<QString> lostFiles = m_lostFilesAbsenceCounter.keys();
    Q_FOREACH (const QString &path, lostFiles) {
        slotFileChanged(path);
    }
}

QString KisFileSystemWatcherWrapper::unifyFilePath(const QString &path)
{
    return QFileInfo(path).absoluteFilePath();
}
