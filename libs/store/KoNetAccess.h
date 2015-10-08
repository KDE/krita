/*
    This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)
    Copyright (C) 1998 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 1999-2004 David Faure (faure@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KO_NETACCESS_h
#define KO_NETACCESS_h

#include <QtCore/QObject>
#include <QtCore/QString>
#include <kio/global.h>
#include <kio/udsentry.h>
#include <kio/jobclasses.h> // for KIO::JobFlags

class QStringList;
class QWidget;

#include <kostore_export.h>


template<typename T, typename K> class QMap;

class KJob;
namespace KIO
{

class Job;

class NetAccessPrivate;
/**
 * Net Transparency.
 *
 * NetAccess allows you to do simple file operation (load, save,
 * copy, delete...) without working with KIO::Job directly.
 * Whereas a KIO::Job is asynchronous, meaning that the
 * developer has to connect slots for it, KIO::NetAccess provides
 * synchronous downloads and uploads, as well as temporary file
 * creation and removal. The functions appear to be blocking,
 * but the Qt event loop continues running while the operations
 * are handled. More precisely, the GUI will still repaint, but no user
 * interaction will be possible. If you can, please use async KIO jobs instead!
 * See the documentation of KJob::exec() for more about the dangers of NetAccess.
 *
 * This class isn't meant to be used as a class but only as a simple
 * namespace for static functions, though an instance of the class
 * is built for internal purposes. TODO KDE5: turn into namespace,
 * and make the qobject class private.
 *
 * Port to kio done by David Faure, faure@kde.org
 *
 * @short Provides a blocking interface to KIO file operations.
 */
class KOSTORE_EXPORT NetAccess : public QObject
{
    Q_OBJECT

public:
    enum StatSide {
        SourceSide,
        DestinationSide
    };

    /**
     * Downloads a file from an arbitrary URL (@p src) to a
     * temporary file on the local filesystem (@p target).
     *
     * If the argument
     * for @p target is an empty string, download will generate a
     * unique temporary filename in /tmp. Since @p target is a reference
     * to QString you can access this filename easily. Download will
     * return true if the download was successful, otherwise false.
     *
     * Special case:
     * If the URL is of kind file:, then no downloading is
     * processed but the full filename is returned in @p target.
     * That means you @em have to take care about the @p target argument.
     * (This is very easy to do, please see the example below.)
     *
     * Download is synchronous. That means you can use it like this:
     * (assuming your application has a loadFile() function)
     *
     * \code
     * QString tmpFile;
     * if( KIO::NetAccess::download(url, tmpFile, window)) {
     *     loadFile(tmpFile);
     *     KIO::NetAccess::removeTempFile(tmpFile);
     * } else {
     *     KMessageBox::error(this, KIO::NetAccess::lastErrorString());
     * }
     * \endcode
     *
     * Of course, your user interface will still process exposure/repaint
     * events during the download.
     *
     * If the download fails, lastError() and lastErrorString() will be set.
     *
     * If the url is always remote, then you could also just write the more usual way:
     * \code
     * QTemporaryFile tmpFile;
     * if (tmpFile.open()) {
     *     KIO::Job* getJob = KIO::file_copy(url, QUrl::fromLocalFile(tmpFile.fileName()), -1, KIO::Overwrite | KIO::HideProgressInfo);
     *     getJob->ui()->setWindow(window);
     *     if (KIO::NetAccess::synchronousRun(getJob, 0)) {
     *         loadFile(tmpFile.fileName());
     *     } else {
     *         getJob->ui()->showErrorMessage();
     *     }
     * }
     * \endcode
     *
     * @param src URL Reference to the file to download.
     * @param target String containing the final local location of the
     *               file.  If you insert an empty string, it will
     *               return a location in a temporary spot. <B>Note:</B>
     *               you are responsible for the removal of this file when
     *               you are finished reading it using removeTempFile.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return true if successful, false for failure.  Use lastErrorString() to
     *         get the reason it failed.
     *
     * @see lastErrorString()
     * @deprecated since 5.0, use KIO::storedGet + KJobWidgets::setWindow + job->exec() instead
     */
    static bool download(const QUrl &src, QString &target, QWidget *window);

    /**
     * Removes the specified file if and only if it was created
     * by KIO::NetAccess as a temporary file for a former download.
     *
     * Note: This means that if you created your temporary with KTempFile,
     * use KTempFile::unlink() or KTempFile::setAutoDelete() to have
     * it removed.
     *
     * @param name Path to temporary file to remove.  May not be
     *             empty.
     */
    static void removeTempFile(const QString &name);

    /**
     * Uploads file @p src to URL @p target.
     *
     * Both must be specified, unlike download.
     * Note that this is assumed to be used for saving a file over
     * the network, so overwriting is set to true. This is not the
     * case with copy.
     *
     * @param src URL Referencing the file to upload.
     * @param target URL containing the final location of the file.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be cached
     *               only for a short duration after which the user will again be
     *               prompted for passwords as needed.
     *
     * @return true if successful, false for failure
     * @deprecated since 5.0, use KIO::storedPut + KJobWidgets::setWindow + job->exec() instead
     */
    static bool upload(const QString &src, const QUrl &target, QWidget *window);

    /**
     * Alternative to upload for copying over the network.
     * Overwrite is false, so this will fail if @p target exists.
     *
     * This one takes two URLs and is a direct equivalent of KIO::file_copy.
     *
     * @param src URL Referencing the file to upload.
     * @param target URL containing the final location of the file.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be cached
     *               only for a short duration after which the user will again be
     *               prompted for passwords as needed.
     *
     * @return true if successful, false for failure
     * @deprecated since 5.0, use KIO::file_copy + job->ui()->setWindow() + job->exec() instead
     */
    static  bool file_copy(const QUrl &src, const QUrl &target, QWidget *window = 0);
    static  bool copy(const QUrl &src, const QUrl &target,
            QWidget *window = 0);

    /**
     * Alternative method for copying over the network.
     *
     * This one takes two URLs and is a direct equivalent
     * of KIO::copy!.
     * This means that it can copy files and directories alike
     * (it should have been named copy()).
     *
     * This method will bring up a dialog if the destination already exists.
     *
     * @param src URL Referencing the file to upload.
     * @param target URL containing the final location of the
     *               file.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be cached
     *               only for a short duration after which the user will again be
     *               prompted for passwords as needed.
     * @return true if successful, false for failure
     * @deprecated since 5.0, use KIO::copy + job->ui()->setWindow() + job->exec() instead
     */
    static  bool dircopy(const QUrl &src, const QUrl &target, QWidget *window);

    /**
     * Overloaded method, which takes a list of source URLs
     * @deprecated since 5.0, use KIO::copy + job->ui()->setWindow() + job->exec() instead
     */
    static   bool dircopy(const QList<QUrl> &src, const QUrl &target, QWidget *window = 0L);

    /**
     * Full-fledged equivalent of KIO::move.
     * Moves or renames one file or directory.
     * @deprecated since 5.0, use KIO::move + job->ui()->setWindow() + job->exec() instead
     */
    static  bool move(const QUrl &src, const QUrl &target, QWidget *window = 0L);

    /**
     * Full-fledged equivalent of KIO::move.
     * Moves or renames a list of files or directories.
     * @deprecated since 5.0, use KIO::move + job->ui()->setWindow() + job->exec() instead
     */
    static  bool move(const QList<QUrl> &src, const QUrl &target, QWidget *window = 0L);

    /**
     * Tests whether a URL exists.
     *
     * @param url the URL we are testing
     * @param source if true, we want to read from that URL.
     *               If false, we want to write to it.
     * IMPORTANT: see documentation for KIO::stat for more details about this.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return true if the URL exists and we can do the operation specified by
     *              @p source, false otherwise
     *
     * @deprecated use the StatSide enum instead of the bool source
     */
    static  bool exists(const QUrl &url, bool source, QWidget *window);

    /**
     * Tests whether a URL exists.
     *
     * @param url the URL we are testing
     * @param statSide determines if we want to read or write.
     * IMPORTANT: see documentation for KIO::stat for more details about this.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return true if the URL exists and we can do the operation specified by
     *              @p source, false otherwise
     * @deprecated since 5.0, use QFile::exists for local files and KIO::stat for transparency
     */
    static bool exists(const QUrl &url, StatSide statSide, QWidget *window);

    /**
     * Tests whether a URL exists and return information on it.
     *
     * This is a convenience function for KIO::stat
     * (it saves creating a slot and testing for the job result).
     *
     * @param url The URL we are testing.
     * @param entry The result of the stat. Iterate over the list
     * of atoms to get hold of name, type, size, etc., or use KFileItem.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return true if successful, false for failure
     * @deprecated since 5.0, use KIO::stat + KJobWidgets::setWindow + job->exec() instead
     */
    static bool stat(const QUrl &url, KIO::UDSEntry &entry, QWidget *window);

    /**
     * Tries to map a local URL for the given URL.
     *
     * This is a convenience function for KIO::stat + parsing the
     * resulting UDSEntry.
     *
     * @param url The URL we are testing.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return a local URL corresponding to the same resource than the
     *         original URL, or the original URL if no local URL can be mapped
     * @deprecated since 5.0, use KIO::mostLocalUrl + KJobWidgets::setWindow + job->exec() instead
     */
    static QUrl mostLocalUrl(const QUrl &url, QWidget *window);

    /**
     * Deletes a file or a directory in a synchronous way.
     *
     * This is a convenience function for KIO::del
     * (it saves creating a slot and testing for the job result).
     *
     * @param url The file or directory to delete.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return true on success, false on failure.
     * @deprecated since 5.0, use KIO::del + job->ui()->setWindow() + job->exec() instead
     */
    static  bool del(const QUrl &url, QWidget *window);

    /**
     * Creates a directory in a synchronous way.
     *
     * This is a convenience function for @p KIO::mkdir
     * (it saves creating a slot and testing for the job result).
     *
     * @param url The directory to create.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @param permissions directory permissions.
     * @return true on success, false on failure.
     * @deprecated since 5.0, use KIO::mkdir + job->ui()->setWindow() + job->exec() instead
     */
    static  bool mkdir(const QUrl &url, QWidget *window, int permissions = -1);

    /**
     * Executes a remote process via the fish ioslave in a synchronous way.
     *
     * @param url The remote machine where the command should be executed.
     *            e.g. fish://someuser\@somehost:sshport/
     *            some special cases exist.
     *            fish://someuser\@localhost/
     *            will use su instead of ssh to connect and execute the command.
     *            fish://someuser\@localhost:port/
     *            will use ssh to connect and execute the command.
     * @param command The command to be executed.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return The resulting output of the @p command that is executed.
     */
    static QString fish_execute(const QUrl &url, const QString &command, QWidget *window);

    /**
     * This function executes a job in a synchronous way.
     * If a job fetches some data, pass a QByteArray pointer as data parameter to this function
     * and after the function returns it will contain all the data fetched by this job.
     *
     * @code
     * KIO::Job *job = KIO::get( url );
     * QMap<QString, QString> metaData;
     * metaData.insert( "PropagateHttpHeader", "true" );
     * if ( NetAccess::synchronousRun( job, 0, &data, &url, &metaData ) ) {
     *   QString responseHeaders = metaData[ "HTTP-Headers" ];
     *   qDebug()<<"Response header = "<< responseHeaders;
     * }
     * @endcode
     *
     * @param job job which the function will run. Note that after this function
     *            finishes running, job is deleted and you can't access it anymore!
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @param data if passed and relevant to this job then it will contain the data
     *               that was fetched by the job
     * @param finalURL if passed will contain the final url of this job (it might differ
     *                 from the one it was created with if there was a redirection)
     * @param metaData you can pass a pointer to the map with meta data you wish to
     *                 set on the job. After the job finishes this map will hold all the
     *                 meta data from the job.
     *
     * @return true on success, false on failure.
     * @deprecated since 5.0, KJobWidgets::setWindow + job->exec() instead
     */
    static bool synchronousRun(Job *job, QWidget *window, QByteArray *data = 0,
                               QUrl *finalURL = 0, QMap<QString, QString> *metaData = 0);

    /**
     * Determines the mimetype of a given URL.
     *
     * This is a convenience function for KIO::mimetype.  You
     * should call this only when really necessary.
     * QMimeDatabase::mimeTypeForUrl can determine the mimetype a lot faster, but
     * less reliably for remote files. When mimeTypeForUrl() returns
     * unknown (application/octet-stream) for a remote file, then this one
     * should be used.
     *
     * @param url The URL whose mimetype we are interested in.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return The mimetype name.
     * @deprecated since 5.0, use KIO::mimetype + KJobWidgets::setWindow + job->exec() instead
     */
    static QString mimetype(const QUrl &url, QWidget *window);

    /**
     * Returns the error string for the last job, in case it failed.
     * Note that this is already translated.
     * @return the last error string, or QString()
     */
    static QString lastErrorString();

    /**
     * Returns the error code for the last job, in case it failed.
     * @return the last error code
     */
    static int lastError();

Q_SIGNALS:
    void leaveModality();
private:
    /**
     * Private constructor
     */
    NetAccess();

    /**
     * Private destructor
     */
    ~NetAccess();

    /**
     * Internal methods
     */
    bool filecopyInternal(const QUrl &src, const QUrl &target, int permissions,
                          KIO::JobFlags flags, QWidget *window, bool move);
    bool dircopyInternal(const QList<QUrl> &src, const QUrl &target,
                         QWidget *window, bool move);
    bool statInternal(const QUrl &url, int details, StatSide side, QWidget *window = 0);

    bool delInternal(const QUrl &url, QWidget *window = 0);
    bool mkdirInternal(const QUrl &url, int permissions, QWidget *window = 0);
    QString fish_executeInternal(const QUrl &url, const QString &command, QWidget *window = 0);
    bool synchronousRunInternal(Job *job, QWidget *window, QByteArray *data,
                                QUrl *finalURL, QMap<QString, QString> *metaData);

    QString mimetypeInternal(const QUrl &url, QWidget *window = 0);
    void enter_loop();

    friend class I_like_this_class;

private Q_SLOTS:
    void slotResult(KJob *job);
    void slotMimetype(KIO::Job *job, const QString &type);
    void slotData(KIO::Job *, const QByteArray &);
    void slotRedirection(KIO::Job *, const QUrl &);

private:
    NetAccessPrivate *const d;
};

}

#endif
