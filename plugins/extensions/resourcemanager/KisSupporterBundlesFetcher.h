/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef __KISSUPPORTERBUNDLESFETCHER_H_
#define __KISSUPPORTERBUNDLESFETCHER_H_

#include <QFileInfo>
#include <QObject>
#include <QUrl>
#include <QVector>
#include <functional>

#include "KisSupporterBundle.h"

class KisNetworkAccessManager;
class QNetworkReply;
class QSqlDatabase;

class KisSupporterBundlesFetcher : public QObject
{
    Q_OBJECT
public:
    explicit KisSupporterBundlesFetcher(QObject *parent = nullptr);

    bool isFetching() const
    {
        return m_fetching;
    }

    const QVector<KisSupporterBundle> &bundles() const
    {
        return m_bundles;
    }

    void fetchIndex();

    QNetworkReply *fetchBundle(const KisSupporterBundle &bundle);

    static bool hasNetworkReplyError(QNetworkReply *reply, QString &outErrorMessage);

Q_SIGNALS:
    void fetchIndexDone(bool success, const QString &errorMessage);

private:
    static constexpr char BASE_URL[] = "https://cdn.kde.org/krita/.android-supporters-extra/supporterbundles/";

    void handleCheckTxtFetched(QNetworkReply *reply);
    bool handleCheckTxtPredefinedFailure(const QString &content);

    bool checkExistingIndexDb(unsigned int expectedVersion, const QString &expectedHash);

    void fetchIndexDb(unsigned int expectedVersion, const QString &expectedHash);

    void handleIndexDbFetched(QNetworkReply *reply, unsigned int expectedVersion, const QString &expectedHash);

    void readIndexDbBundles();

    QNetworkReply *fetch(const QString &path);

    void emitCheckTxtFetchFailed(const QString &errorMessage);
    void emitIndexDbFetchFailed(const QString &errorMessage);
    void emitFailure(const QString &errorMessage);

    bool doWithIndexDb(QString &outErrorMessage, const std::function<bool(QSqlDatabase &, QString &)> &block);

    static bool readSqliteUserVersion(const QByteArray &bytes, unsigned int &outVersion);

    KisNetworkAccessManager *m_nam = nullptr;
    QString m_indexDbPath;
    QVector<KisSupporterBundle> m_bundles;
    bool m_fetching = false;
};

#endif
