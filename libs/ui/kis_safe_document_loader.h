/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SAFE_DOCUMENT_LOADER_H
#define __KIS_SAFE_DOCUMENT_LOADER_H

#include <QObject>
#include "kis_types.h"

class KisSafeDocumentLoader : public QObject
{
    Q_OBJECT
private:
    friend class KisFileLayer;

    KisSafeDocumentLoader(const QString &path = "", QObject *parent = 0);
    ~KisSafeDocumentLoader() override;

    void setPath(const QString &path);
    void reloadImage();
private Q_SLOTS:
    void fileChanged(QString);
    void fileChangedCompressed(bool sync = false);
    void delayedLoadStart();

Q_SIGNALS:
    void loadingFinished(KisPaintDeviceSP paintDevice, qreal xRes, qreal yRes, const QSize &size);
    void loadingFailed();

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_SAFE_DOCUMENT_LOADER_H */
