/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
