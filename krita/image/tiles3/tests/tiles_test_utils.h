/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TILES_TEST_UTILS_H
#define TILES_TEST_UTILS_H

#include <KoStore_p.h>
#include <kis_paint_device_writer.h>

class KisFakePaintDeviceWriter : public KisPaintDeviceWriter {
public:
    KisFakePaintDeviceWriter(KoStore *store)
        : m_store(store)
    {
    }

    qint64 write(const QByteArray &data) {
        return m_store->write(data);
    }

    qint64 write(const char* data, qint64 length) {
        return m_store->write(data, length);
    }

    KoStore *m_store;
};


class KoStoreFake : public KoStore
{
public:
    KoStoreFake() {
        d_ptr->stream = &m_buffer;
        d_ptr->isOpen = true;
        d_ptr->mode = KoStore::Write;
        m_buffer.open(QIODevice::ReadWrite);
    }
    ~KoStoreFake() {
        // Oh, no, please do not clean anything! :)
        d_ptr->stream = 0;
        d_ptr->isOpen = false;
    }

    void startReading() {
        m_buffer.seek(0);
        d_ptr->mode = KoStore::Read;
    }

    bool openWrite(const QString&) { return true; }
    bool openRead(const QString&) { return true; }
    bool closeRead() { return true; }
    bool closeWrite() { return true; }
    bool enterRelativeDirectory(const QString&) { return true; }
    bool enterAbsoluteDirectory(const QString&) { return true; }
    bool fileExists(const QString&) const { return true; }
private:
    QBuffer m_buffer;
};

bool memoryIsFilled(quint8 c, quint8 *mem, qint32 size)
{
    for(; size > 0; size--)
        if(*(mem++) != c) {
            qDebug() << "Expected" << c << "but found" << *(mem-1);
            return false;
        }

    return true;
}

#define TILESIZE 64*64


#endif /* TILES_TEST_UTILS_H */
