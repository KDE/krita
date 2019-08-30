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

class DummyResource : public KoResource {
public:
    DummyResource(const QString &f) : KoResource(f) {}

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

private:

    QString m_something;
};

#endif // DUMMYRESOURCE_H
