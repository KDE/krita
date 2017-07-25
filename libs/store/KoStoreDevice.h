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

#include "KoStore.h"
#include <kritastore_export.h>


/**
 * This class implements a QIODevice around KoStore, so that
 * it can be used to create a QDomDocument from it, to be written or read
 * using QDataStream or to be written using QTextStream
 */
class KRITASTORE_EXPORT KoStoreDevice : public QIODevice
{
    Q_OBJECT
public:
    /// Note: KoStore::open() should be called before calling this.
    explicit KoStoreDevice(KoStore * store) : m_store(store) {
        // calligra-1.x behavior compat: a KoStoreDevice is automatically open
        setOpenMode(m_store->mode() == KoStore::Read ? QIODevice::ReadOnly : QIODevice::WriteOnly);
    }
    ~KoStoreDevice() override;

    bool isSequential() const override {
        return true;
    }

    bool open(OpenMode m) override {
        setOpenMode(m);
        if (m & QIODevice::ReadOnly)
            return (m_store->mode() == KoStore::Read);
        if (m & QIODevice::WriteOnly)
            return (m_store->mode() == KoStore::Write);
        return false;
    }
    void close() override {}

    qint64 size() const override {
        if (m_store->mode() == KoStore::Read)
            return m_store->size();
        else
            return 0xffffffff;
    }

    // See QIODevice
    qint64 pos() const override {
        return m_store->pos();
    }
    bool seek(qint64 pos) override {
        return m_store->seek(pos);
    }
    bool atEnd() const override {
        return m_store->atEnd();
    }

protected:
    KoStore *m_store;

    qint64 readData(char *data, qint64 maxlen) override {
        return m_store->read(data, maxlen);
    }

    qint64 writeData(const char *data, qint64 len) override {
        return m_store->write(data, len);
    }

};

#endif
