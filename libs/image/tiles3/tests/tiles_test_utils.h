/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TILES_TEST_UTILS_H
#define TILES_TEST_UTILS_H

#include <KoStore_p.h>
#include <kis_paint_device_writer.h>
#include <kis_debug.h>

class KisFakePaintDeviceWriter : public KisPaintDeviceWriter {
public:
    KisFakePaintDeviceWriter(KoStore *store)
        : m_store(store)
    {
    }

    bool write(const QByteArray &data) override {
        return (m_store->write(data) == data.length());
    }

    bool write(const char* data, qint64 length) override {
        return (m_store->write(data, length) == length);
    }

    KoStore *m_store;
};


class KoStoreFake : public KoStore
{
public:
    KoStoreFake() : KoStore(KoStore::Write) {
        d_ptr->stream = &m_buffer;
        d_ptr->isOpen = true;
        m_buffer.open(QIODevice::ReadWrite);
    }
    ~KoStoreFake() override {
        // Oh, no, please do not clean anything! :)
        d_ptr->stream = 0;
        d_ptr->isOpen = false;
    }

    void startReading() {
        m_buffer.seek(0);
        d_ptr->mode = KoStore::Read;
    }

    bool openWrite(const QString&) override { return true; }
    bool openRead(const QString&) override { return true; }
    bool closeRead() override { return true; }
    bool closeWrite() override { return true; }
    bool enterRelativeDirectory(const QString&) override { return true; }
    bool enterAbsoluteDirectory(const QString&) override { return true; }
    bool fileExists(const QString&) const override { return true; }
private:
    QBuffer m_buffer;
};

bool memoryIsFilled(quint8 c, quint8 *mem, qint32 size)
{
    for(; size > 0; size--)
        if(*(mem++) != c) {
            dbgKrita << "Expected" << c << "but found" << *(mem-1);
            return false;
        }

    return true;
}

#define TILESIZE 64*64


#endif /* TILES_TEST_UTILS_H */
