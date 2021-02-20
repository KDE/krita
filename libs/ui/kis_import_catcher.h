/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_IMPORT_CATCHER_H_
#define KIS_IMPORT_CATCHER_H_

#include <QObject>

#include <kis_types.h>

class KisViewManager;
class QUrl;

/**
 * This small helper class takes an url and an image; tries to import
 * the image at the url and shove the layers of the imported image
 * into the first image after loading is done. This is a separate class
 * because loading can be asynchronous.
 *
 * Caveat: this class calls "delete this", which means that you new
 * it and then never touch it again. Thank you very much.
 */
class KisImportCatcher : QObject
{

    Q_OBJECT

public:

    KisImportCatcher(const QUrl &url, KisViewManager* view, const QString &layerType);
    ~KisImportCatcher() override;

    static void adaptClipToImageColorSpace(KisPaintDeviceSP dev, KisImageSP image);

private Q_SLOTS:
    void slotLoadingFinished();

private:
    void deleteMyself();

private:

    struct Private;
    Private* const m_d;
};

#endif
