/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_SVG_IMPORT_H_
#define _KIS_SVG_IMPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class KisSVGImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisSVGImport(QObject *parent, const QVariantList &);
    ~KisSVGImport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration) override;
};

#endif
