//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
// Copyright 2009      Jens-Michael Hoffmann <jensmh@gmx.de>
//

//
// The HttpDownloadManager manages http downloads.
//

#ifndef MARBLE_HTTPDOWNLOADMANAGER_H
#define MARBLE_HTTPDOWNLOADMANAGER_H

#include <QObject>

#include <marble/MarbleGlobal.h>
#include <marble/marble_export.h>

class QUrl;

namespace Marble
{

class DownloadPolicy;
class DownloadQueueSet;
class StoragePolicy;

/**
 * @Short This class manages scheduled downloads. 

 * The downloadmanager offers a maximum number of active jobs and a
 * limit for pending jobs.  it also takes care that the job queue
 * won't be polluted by jobs that timed out already.
 *
 * @author Torsten Rahn
 */

class MARBLE_EXPORT HttpDownloadManager : public QObject 
{
    Q_OBJECT

 public:
    /**
     * Creates a new http download manager.
     *
     * @note HttpDownloadManager doesn't take ownership of @p policy.
     *
     * @param policy The storage policy for this manager.
     */
    explicit HttpDownloadManager( StoragePolicy *policy );

    /**
     * Destroys the http download manager.
     */
    virtual ~HttpDownloadManager();

    /**
     * Switches loading on/off, useful for offline mode.
     */
    void setDownloadEnabled( const bool enable );
    void addDownloadPolicy( const DownloadPolicy& );

 public Q_SLOTS:

    /**
     * Adds a new job with a sourceUrl, destination file name and given id.
     */
    void addJob( const QUrl& sourceUrl, const QString& destFilename, const QString &id,
                 const DownloadUsage usage );


 Q_SIGNALS:
    void downloadComplete( QString, QString );

    /**
     * This signal is emitted if a file is downloaded and the data argument
     * contains the files content. The HttpDownloadManager takes care to save
     * it using the given storage policy.
     */
    void downloadComplete( QByteArray data, QString initiatorId );

    /**
     * Signal is emitted when a new job is added to the queue.
     */
    void jobAdded();

    /**
     * Signal is emitted when a job is removed from the queue.
     * The job might be completed or blacklisted.
     */
    void jobRemoved();

    /**
      * A job was queued, activated or removed (finished, failed)
      */
    void progressChanged( int active, int queued );

 private Q_SLOTS:
    void finishJob( const QByteArray& data, const QString& destinationFileName,
		    const QString& id );
    void requeue();
    void startRetryTimer();

 private:
    Q_DISABLE_COPY( HttpDownloadManager )

    void connectDefaultQueueSets();
    void connectQueueSet( DownloadQueueSet * );
    bool hasDownloadPolicy( const DownloadPolicy& policy ) const;
    class Private;
    Private * const d;
};

}

#endif // MARBLE_HTTPDOWNLOADMANAGER_H
