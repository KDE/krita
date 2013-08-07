/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */

#ifndef KIS_ANIMATION_STORE_WRITER_H
#define KIS_ANIMATION_STORE_WRITER_H

#include <kis_paint_device_writer.h>
#include "kis_animation_store.h"

class KisAnimationStoreWriter
        : public KisPaintDeviceWriter
{
public:
    KisAnimationStoreWriter(KisAnimationStore* store, QString filename)
        : m_store(store)
        , m_filename(filename)
    {
    }

    qint64 write(const char*, qint64)
    {
        return 1;
    }

    qint64 write(const QByteArray &data)
    {
        m_store->writeDataToFile(data, m_filename);
        return data.length();
    }

    KisAnimationStore* m_store;
    QString m_filename;
};

#endif // KIS_ANIMATION_STORE_WRITER_H
