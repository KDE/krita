/*
 *  SPDX-FileCopyrightText: 2013 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_STORE_PAINTDEVICE_WRITER_H
#define KIS_STORE_PAINTDEVICE_WRITER_H

#include <kis_paint_device_writer.h>
#include <KoStore.h>

class KisStorePaintDeviceWriter : public KisPaintDeviceWriter {
public:
    KisStorePaintDeviceWriter(KoStore *store)
        : m_store(store)
    {
    }

    ~KisStorePaintDeviceWriter() override {}

    bool write(const QByteArray &data) override {
        qint64 len = m_store->write(data);
        return (len == data.size());
    }

    bool write(const char* data, qint64 length) override {
        qint64 len = m_store->write(data, length);
        return (length == len);
    }

    KoStore *m_store;

};

#endif // KIS_STORE_PAINTDEVICE_WRITER_H
