/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _QML_EXPORT_H_
#define _QML_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class QMLExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    QMLExport(QObject *parent, const QVariantList &);
    ~QMLExport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    void initializeCapabilities() override;
};

#endif
