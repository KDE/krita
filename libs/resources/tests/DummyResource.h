/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
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
#ifndef DUMMYRESOURCE_H
#define DUMMYRESOURCE_H

#include "KoResource.h"
#include <QDebug>
#include <QRandomGenerator64>
#include <KoHashGenerator.h>
#include <KoHashGeneratorProvider.h>
#include <KisResourceTypes.h>

class DummyResource : public KoResource {
public:
    DummyResource(const QString &f, const QString &resourceType = ResourceType::PaintOpPresets)
        : KoResource(f)
        , m_resourceType(resourceType)
    {
        QRandomGenerator64 qrg;
        QByteArray ba(1024, '0');
        for (int i = 0; i < 1024 / 8; i+=8) {
            quint64 v = qrg.generate64();
            ba[i] = v;
        }
        KoHashGenerator *hashGenerator = KoHashGeneratorProvider::instance()->getGenerator("MD5");
        QByteArray hash = hashGenerator->generateHash(ba);
        setMD5(hash);
        setValid(true);
    }

    DummyResource(const DummyResource &rhs)
        : KoResource(rhs)
    {
        *this = rhs;
    }

    DummyResource &operator=(const DummyResource &rhs)
    {
        if (*this != rhs) {
            m_something = rhs.m_something;
        }
        return *this;
    }

    KoResourceSP clone() const override
    {
        return KoResourceSP(new DummyResource(*this));
    }

    bool load() override
    {
        Q_ASSERT(false);
        setValid(true);
        return true;
    }

    bool loadFromDevice(QIODevice *dev) override
    {
        if (!dev->isOpen()) {
            dev->open(QIODevice::ReadOnly);
        }
        m_something = QString::fromUtf8(dev->readAll());
        setValid(true);
        return true;
    }

    bool save() override
    {
        Q_ASSERT(false);
        return true;
    }

    bool saveToDevice(QIODevice *dev) const
    {
        if (!dev->isOpen()) {
            dev->open(QIODevice::WriteOnly);
        }
        dev->write(m_something.toUtf8());
        return true;
    }

    void setSomething(const QString &something)
    {
        m_something = something;
    }

    QString something() const
    {
        return m_something;
    }

    QPair<QString, QString> resourceType() const override {
        return QPair<QString, QString>(m_resourceType, "");
    }

private:

    QString m_something;
    QString m_resourceType;
};

#endif // DUMMYRESOURCE_H
