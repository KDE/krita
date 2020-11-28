/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _QML_CONVERTER_H_
#define _QML_CONVERTER_H_

#include <stdio.h>

#include <QObject>

#include <QFileInfo>

#include "kis_types.h"
#include <KisImportExportErrorCode.h>

class QMLConverter : public QObject
{
    Q_OBJECT
public:
    QMLConverter();
    ~QMLConverter() override;
public:
    KisImportExportErrorCode buildFile(const QString &filename, const QString &realFilename, QIODevice *io, KisImageSP image);

private:
    void writeString(QTextStream& out, int spacing, const QString& setting, const QString& value);
    void writeInt(QTextStream& out, int spacing, const QString& setting, int value);
};

#endif
