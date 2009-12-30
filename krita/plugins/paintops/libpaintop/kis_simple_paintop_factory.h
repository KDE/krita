/*
 * Copyright (c) 2009 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef KIS_SIMPLE_PAINTOP_FACTORY_H
#define KIS_SIMPLE_PAINTOP_FACTORY_H

#include <KoInputDevice.h>
#include <kis_paintop_factory.h>

/**
 *
 */
template <class Op, class OpSettings, class OpSettingsWidget> class KisSimplePaintOpFactory  : public KisPaintOpFactory {

public:
    KisSimplePaintOpFactory(const QString& id, const QString& name, const QString& pixmap)
        : m_id(id)
        , m_name(name)
        , m_pixmap(pixmap)
        {
        }

    virtual ~KisSimplePaintOpFactory()
        {
        }
        
    KisPaintOp * createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageWSP image = 0)
    {
        const OpSettings *opSettings = dynamic_cast<const OpSettings *>(settings.data());
        Q_ASSERT(settings == 0 || opSettings != 0);

        KisPaintOp * op = new Op(opSettings, painter, image);
        Q_CHECK_PTR(op);
        return op;
    }

    KisPaintOpSettingsSP settings(const KoInputDevice& inputDevice, KisImageWSP image)
    {
        Q_UNUSED(inputDevice);
        Q_UNUSED(image);
        return new OpSettings();
    }

    KisPaintOpSettingsSP settings(KisImageWSP image)
    {
        Q_UNUSED(image);
        return new OpSettings();
    }

    KisPaintOpSettingsWidget* createSettingsWidget(QWidget* parent)
    {
        return new OpSettingsWidget(parent);
    }
    
    QString id() const {
        return m_id;
    }

    QString name() const {
        return m_name;
    }

    QString pixmap() {
        return m_pixmap;
    }
    
private:
    QString m_id;
    QString m_name;
    QString m_pixmap;
};

#endif // KIS_SIMPLE_PAINTOP_FACTORY_H
