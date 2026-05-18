/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisSupporterBundlesFetcher.h"
#include <KisNetworkAccessManager.h>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QtEndian>
#include <kis_assert.h>
#include <kis_debug.h>
#include <klocalizedstring.h>

KisSupporterBundlesFetcher::KisSupporterBundlesFetcher(QObject *parent)
    : QObject(parent)
{
    QDir cacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    m_indexDbPath = cacheDir.filePath(QStringLiteral("kissupporterbundlesindex.db"));
    dbgResources << "Index db path:" << m_indexDbPath;
}

void KisSupporterBundlesFetcher::fetchIndex()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_fetching);
    m_fetching = true;
    QNetworkReply *reply = fetch(QStringLiteral("check0.txt"));
    connect(reply, &QNetworkReply::finished, this, [=] {
        handleCheckTxtFetched(reply);
        reply->deleteLater();
    });
}

QNetworkReply *KisSupporterBundlesFetcher::fetchBundle(const KisSupporterBundle &bundle)
{
    return fetch(QStringLiteral("bundles/%1").arg(bundle.fileName));
}

bool KisSupporterBundlesFetcher::hasNetworkReplyError(QNetworkReply *reply, QString &outErrorMessage)
{
    if (reply->error() != QNetworkReply::NoError) {
        outErrorMessage = reply->errorString();
        return true;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode < 200 || statusCode > 300) {
        outErrorMessage = i18n("Server responded with error %1.").arg(statusCode);
        return true;
    }

    return false;
}

void KisSupporterBundlesFetcher::handleCheckTxtFetched(QNetworkReply *reply)
{
    QString errorMessage;
    if (hasNetworkReplyError(reply, errorMessage)) {
        emitCheckTxtFetchFailed(errorMessage);
        return;
    }

    QString content = QString::fromUtf8(reply->readAll());

    // "Predefined failures" are for forward-compatibility for stuff like the
    // format changing and old versions of Krita becoming unsupported or the
    // service getting discontinued altogether.
    if (handleCheckTxtPredefinedFailure(content)) {
        return;
    }

    // The check0.txt is supposed to contain a version followed by an MD5 hash
    // digest. For forward-compatibility, we'll ignore anything else afterwards.
    QRegularExpression re(QStringLiteral("\\A\\s*([0-9]+)\\s+([0-9a-fA-F]+)\\b"));
    QRegularExpressionMatch match = re.match(content);
    if (!match.hasMatch()) {
        emitCheckTxtFetchFailed(i18n("Invalid checksums file."));
        return;
    }

    bool ok;
    unsigned int expectedVersion = match.captured(1).toUInt(&ok);
    dbgResources << "Expected version from fetched check0.txt:" << expectedVersion;
    if (!ok) {
        emitCheckTxtFetchFailed(i18n("Invalid checksums version."));
        return;
    }

    QString expectedHash = match.captured(2);
    dbgResources << "Expected hash from fetched check0.txt:" << expectedHash;

    if (checkExistingIndexDb(expectedVersion, expectedHash)) {
        readIndexDbBundles();
    } else {
        fetchIndexDb(expectedVersion, expectedHash);
    }
}

bool KisSupporterBundlesFetcher::handleCheckTxtPredefinedFailure(const QString &content)
{
    QRegularExpression re(QStringLiteral("\\A!failure=([a-zA-Z0-9]*)"), QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(content);
    if (!match.hasMatch()) {
        return false;
    }

    QString failureType = match.captured(1);
    QString errorMessage;
    if (failureType.compare(QStringLiteral("outdated"), Qt::CaseInsensitive) == 0) {
        errorMessage = i18n("You have to update Krita to use this service.");
    } else if (failureType.compare(QStringLiteral("region"), Qt::CaseInsensitive) == 0) {
        errorMessage = i18n("This service is not available in your region.");
    } else if (failureType.compare(QStringLiteral("unavailable"), Qt::CaseInsensitive) == 0) {
        errorMessage = i18n("This service is currently unavailable.");
    } else if (failureType.compare(QStringLiteral("discontinued"), Qt::CaseInsensitive) == 0) {
        errorMessage = i18n("This service has been discontinued.");
    } else {
        errorMessage = i18n("Unknown service error %1.").arg(failureType);
    }

    emitFailure(errorMessage);
    return true;
}

bool KisSupporterBundlesFetcher::checkExistingIndexDb(unsigned int expectedVersion, const QString &expectedHash)
{
    QFile file(m_indexDbPath);
    if (!file.open(QIODevice::ReadOnly)) {
        dbgResources << "Could not open existing index db";
        return false;
    }

    QByteArray bytes = file.readAll();
    unsigned int version;
    if (!readSqliteUserVersion(bytes, version)) {
        dbgResources << "Could not read existing version";
        return false;
    }

    if (version != expectedVersion) {
        dbgResources << "Existing version" << version << "!=" << expectedVersion;
        return false;
    }

    QString hash = QCryptographicHash::hash(bytes, QCryptographicHash::Md5).toHex();
    if (hash != expectedHash) {
        dbgResources << "Existing hash" << hash << "!=" << expectedHash;
        return false;
    }

    dbgResources << "Cached index database is valid";
    return true;
}

void KisSupporterBundlesFetcher::fetchIndexDb(unsigned int expectedVersion, const QString &expectedHash)
{
    QNetworkReply *reply = fetch(QStringLiteral("index0.db"));
    connect(reply, &QNetworkReply::finished, this, [=] {
        handleIndexDbFetched(reply, expectedVersion, expectedHash);
        reply->deleteLater();
    });
}

void KisSupporterBundlesFetcher::handleIndexDbFetched(QNetworkReply *reply,
                                                      unsigned int expectedVersion,
                                                      const QString &expectedHash)
{
    QString errorMessage;
    if (hasNetworkReplyError(reply, errorMessage)) {
        emitIndexDbFetchFailed(errorMessage);
        return;
    }

    QByteArray bytes = reply->readAll();
    unsigned int version;
    if (!readSqliteUserVersion(bytes, version)) {
        emitIndexDbFetchFailed(i18n("Not enough data."));
        return;
    }

    dbgResources << "got version:" << version;
    dbgResources << "expected:" << expectedVersion;
    if (version != expectedVersion) {
        emitIndexDbFetchFailed(i18n("Invalid version."));
        return;
    }

    QString hash = QCryptographicHash::hash(bytes, QCryptographicHash::Md5).toHex();
    dbgResources << "got hash:" << hash;
    dbgResources << "expected:" << expectedHash;
    if (hash != expectedHash) {
        emitIndexDbFetchFailed(i18n("Invalid checksum."));
        return;
    }

    QString dirName = QFileInfo(m_indexDbPath).path();
    if (!QDir().mkpath(dirName)) {
        emitIndexDbFetchFailed(i18n("Failed to create directory %1.").arg(dirName));
        return;
    }

    QSaveFile saveFile(m_indexDbPath);
    saveFile.setDirectWriteFallback(true);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        emitIndexDbFetchFailed(i18n("Failed to open %1 for writing.").arg(m_indexDbPath));
        return;
    }

    qint64 written = saveFile.write(bytes);
    if (written != qint64(bytes.size()) || !saveFile.commit()) {
        emitIndexDbFetchFailed(i18n("Failed to write %1.").arg(m_indexDbPath));
        return;
    }

    readIndexDbBundles();
}

void KisSupporterBundlesFetcher::readIndexDbBundles()
{
    QString errorMessage;
    bool ok = doWithIndexDb(errorMessage, [this](QSqlDatabase &db, QString &outErrorMessage) {
        QSqlQuery query(db);

        QString selectBundlesSql = QStringLiteral(
            "select bundle_id, file_name, size_in_bytes, source, title,\n"
            "       description, author, license, checksum, thumbnail\n"
            "from bundle");
        if (!query.exec(selectBundlesSql)) {
            outErrorMessage = query.lastError().text();
            return false;
        }

        QHash<long long, KisSupporterBundle> bundlesById;
        while (query.next()) {
            long long id = query.value(0).toLongLong();
            KisSupporterBundle bundle = {
                query.value(1).toString(),
                query.value(2).toLongLong(),
                query.value(3).toString(),
                query.value(4).toString(),
                query.value(5).toString(),
                query.value(6).toString(),
                query.value(7).toString(),
                query.value(8).toString(),
                QPixmap(),
                QSet<QString>(),
                QHash<QString, int>(),
            };
            bundle.thumbnail.loadFromData(query.value(9).toByteArray());
            bundlesById.insert(id, bundle);
        }

        QString selectTagsSql = QStringLiteral("select bundle_id, tag from bundle_tag");
        if (!query.exec(selectTagsSql)) {
            outErrorMessage = query.lastError().text();
            return false;
        }

        while (query.next()) {
            long long id = query.value(0).toLongLong();
            QHash<long long, KisSupporterBundle>::iterator it = bundlesById.find(id);
            if (it != bundlesById.end()) {
                it->tags.insert(query.value(1).toString());
            }
        }

        QString selectResourceCountsSql = QStringLiteral(
            "select bundle_id, media_type, amount\n"
            "from bundle_resource_count");
        if (query.exec(selectResourceCountsSql)) {
            while (query.next()) {
                long long id = query.value(0).toLongLong();
                QHash<long long, KisSupporterBundle>::iterator it = bundlesById.find(id);
                if (it != bundlesById.end()) {
                    it->resourceCountByMediaType.insert(query.value(1).toString(), query.value(2).toInt());
                }
            }
        } else {
            warnResources << "Error querying bundle resource counts:" << query.lastError().text();
            // Not critical to have these counts, just keep going.
        }

        m_bundles = QVector<KisSupporterBundle>(bundlesById.begin(), bundlesById.end());
        dbgResources << "Loaded" << m_bundles.size() << "supporter bundle(s)";
        return true;
    });

    m_fetching = false;
    emit fetchIndexDone(ok, errorMessage);
}

QNetworkReply *KisSupporterBundlesFetcher::fetch(const QString &path)
{
    QUrl url(QString::fromUtf8(BASE_URL));
    url.setPath(url.path(QUrl::FullyDecoded) + path);

    QNetworkRequest req;
    req.setUrl(url);

    if (!m_nam) {
        m_nam = new KisNetworkAccessManager(this);
    }

    dbgResources << "Fetch" << url;
    return m_nam->get(req);
}

void KisSupporterBundlesFetcher::emitCheckTxtFetchFailed(const QString &errorMessage)
{
    emitFailure(i18n("Failed to retrieve bundle checksums: %1", errorMessage));
}

void KisSupporterBundlesFetcher::emitIndexDbFetchFailed(const QString &errorMessage)
{
    emitFailure(i18n("Failed to retrieve bundles: %1", errorMessage));
}

void KisSupporterBundlesFetcher::emitFailure(const QString &errorMessage)
{
    m_fetching = false;
    Q_EMIT fetchIndexDone(false, errorMessage);
}

bool KisSupporterBundlesFetcher::doWithIndexDb(QString &outErrorMessage,
                                               const std::function<bool(QSqlDatabase &, QString &)> &block)
{
    QString connectionName = QStringLiteral("KisSupporterBundlesFetcher");
    bool ok;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
        db.setDatabaseName(m_indexDbPath);
        if (db.open()) {
            ok = block(db, outErrorMessage);
            db.close();
        } else {
            ok = false;
            outErrorMessage = db.lastError().text();
        }
    }
    QSqlDatabase::removeDatabase(connectionName);
    return ok;
}

bool KisSupporterBundlesFetcher::readSqliteUserVersion(const QByteArray &bytes, unsigned int &outVersion)
{
    // The SQLite format stores its "user version" value at index 60 as a 32 bit
    // bigendian unsigned integer. See https://www.sqlite.org/fileformat.html
    if (bytes.size() < 64) {
        return false;
    } else {
        outVersion = qFromBigEndian<quint32>(bytes.constData() + 60);
        return true;
    }
}
