/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _EXR_CONVERTER_H_
#define _EXR_CONVERTER_H_

#include <stdio.h>

#include <QObject>

#include "kis_types.h"
#include <KisImportExportErrorCode.h>

class KisDocument;

class EXRConverter : public QObject
{
    Q_OBJECT
public:
    EXRConverter(KisDocument *doc, bool showNotifications);
    ~EXRConverter() override;
public:
    KisImportExportErrorCode buildImage(const QString &filename);
    KisImportExportErrorCode buildFile(const QString &filename, KisPaintLayerSP layer);
    KisImportExportErrorCode buildFile(const QString &filename, KisGroupLayerSP layer, bool flatten=false);
    /**
     * Retrieve the constructed image
     */
    KisImageSP image();
    QString errorMessage() const;
private:
    KisImportExportErrorCode decode(const QString &filename);

public Q_SLOTS:
    virtual void cancel();
private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif
