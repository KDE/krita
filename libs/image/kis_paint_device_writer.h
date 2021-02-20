/*
 *  SPDX-FileCopyrightText: 2013 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_PAINT_DEVICE_WRITER_H
#define KIS_PAINT_DEVICE_WRITER_H

#include <kritaimage_export.h>

class KRITAIMAGE_EXPORT KisPaintDeviceWriter {
public:
    virtual ~KisPaintDeviceWriter() {}
    virtual bool write(const QByteArray &data) = 0;
    virtual bool write(const char* data, qint64 length) = 0;
};


#endif // KIS_PAINT_DEVICE_WRITER_H
