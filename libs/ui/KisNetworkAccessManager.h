/*
 * SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <QNetworkAccessManager>

class QUrl;

#include <kritaui_export.h>

/**
 * @brief Network Access Manager for use with Krita.
 */
class KRITAUI_EXPORT KisNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    KisNetworkAccessManager(QObject *parent = 0);

public Q_SLOTS:
    void getUrl(const QUrl &url);

protected:
    QNetworkReply* createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData) override;
};
