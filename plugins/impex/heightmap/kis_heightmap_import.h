/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_HeightMap_IMPORT_H_
#define _KIS_HeightMap_IMPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class KisHeightMapImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisHeightMapImport(QObject *parent, const QVariantList &);
    ~KisHeightMapImport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
};

#endif
