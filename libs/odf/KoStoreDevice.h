/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef koStoreDevice_h
#define koStoreDevice_h

#include <KoStore.h>

/**
 * This class implements a QIODevice around KoStore, so that
 * it can be used to create a QDomDocument from it, to be written or read
 * using QDataStream or to be written using QTextStream
 */
class KoStoreDevice : public QIODevice
{
public:
    /// Note: KoStore::open() should be called before calling this.
    explicit KoStoreDevice(KoStore * store) : m_store(store) {
        // koffice-1.x behavior compat: a KoStoreDevice is automatically open
        setOpenMode(m_store->mode() == KoStore::Read ? QIODevice::ReadOnly : QIODevice::WriteOnly);
    }
    ~KoStoreDevice() {}

    virtual bool isSequential() const {
        return true;
    }

    virtual bool open(OpenMode m) {
        setOpenMode(m);
        if (m & QIODevice::ReadOnly)
            return (m_store->mode() == KoStore::Read);
        if (m & QIODevice::WriteOnly)
            return (m_store->mode() == KoStore::Write);
        return false;
    }
    virtual void close() {}

    qint64 size() const {
        if (m_store->mode() == KoStore::Read)
            return m_store->size();
        else
            return 0xffffffff;
    }

    // See QIODevice
    virtual qint64 pos() const {
        return m_store->pos();
    }
    virtual bool seek(qint64 pos) {
        return m_store->seek(pos);
    }
    virtual bool atEnd() const {
        return m_store->atEnd();
    }

protected:
    KoStore *m_store;

    virtual qint64 readData(char *data, qint64 maxlen) {
        return m_store->read(data, maxlen);
    }

    virtual qint64 writeData(const char *data, qint64 len) {
        return m_store->write(data, len);
    }

};

#endif
