/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_asl_writer_utils.h"

#include <QUuid>
#include <resources/KoPattern.h>

namespace KisAslWriterUtils
{
void writeRect(const QRect &rect, QIODevice *device)
{
    {
        const quint32 rectY0 = rect.y();
        SAFE_WRITE_EX(device, rectY0);
    }
    {
        const quint32 rectX0 = rect.x();
        SAFE_WRITE_EX(device, rectX0);
    }
    {
        const quint32 rectY1 = rect.y() + rect.height();
        SAFE_WRITE_EX(device, rectY1);
    }
    {
        const quint32 rectX1 = rect.x() + rect.width();
        SAFE_WRITE_EX(device, rectX1);
    }
}

void writeUnicodeString(const QString &value, QIODevice *device)
{
    quint32 len = value.length() + 1;
    SAFE_WRITE_EX(device, len);

    const quint16 *ptr = value.utf16();
    for (quint32 i = 0; i < len; i++) {
        SAFE_WRITE_EX(device, ptr[i]);
    }
}

void writeVarString(const QString &value, QIODevice *device)
{
    quint32 lenTag = value.length() != 4 ? value.length() : 0;
    SAFE_WRITE_EX(device, lenTag);

    if (!device->write(value.toLatin1().data(), value.length())) {
        warnKrita << "WARNING: ASL: Failed to write ASL string" << ppVar(value);
        return;
    }
}

void writePascalString(const QString &value, QIODevice *device)
{
    quint8 lenTag = value.length();
    SAFE_WRITE_EX(device, lenTag);

    if (!device->write(value.toLatin1().data(), value.length())) {
        warnKrita << "WARNING: ASL: Failed to write ASL string" << ppVar(value);
        return;
    }
}

void writeFixedString(const QString &value, QIODevice *device)
{
    KIS_ASSERT_RECOVER_RETURN(value.length() == 4);

    if (!device->write(value.toLatin1().data(), value.length())) {
        warnKrita << "WARNING: ASL: Failed to write ASL string" << ppVar(value);
        return;
    }
}

// Write UUID fetched from the file name or generate
QString getPatternUuidLazy(const KoPatternSP pattern)
{
    QUuid uuid;
    QString patternFileName = pattern->filename();

    if (patternFileName.endsWith(".pat", Qt::CaseInsensitive)) {
        QString strUuid = patternFileName.left(patternFileName.size() - 4);

        uuid = QUuid(strUuid);
    }

    if (uuid.isNull()) {
        warnKrita << "WARNING: Saved pattern doesn't have a UUID, generating...";
        warnKrita << ppVar(patternFileName) << ppVar(pattern->name());
        uuid = QUuid::createUuid();
    }

    return uuid.toString().mid(1, 36);
}

}
