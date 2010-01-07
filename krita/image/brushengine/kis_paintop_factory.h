/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_PAINTOP_FACTORY_H_
#define KIS_PAINTOP_FACTORY_H_

#include "kis_types.h"
#include "krita_export.h"
#include <QObject>
#include <QString>

class KoColorSpace;
class KisPainter;
class KisPaintOp;
class KoInputDevice;
class QWidget;
class KisPaintOpSettingsWidget;

/**
 * The paintop factory is responsible for creating paintops of the specified class.
 * If there is an optionWidget, the derived paintop itself must support settings,
 * and it's up to the factory to do that.
 */
class KRITAIMAGE_EXPORT KisPaintOpFactory : public QObject
{
    Q_OBJECT

public:

    KisPaintOpFactory();
    virtual ~KisPaintOpFactory() {}

    /**
     * Create a KisPaintOp with the given settings and painter.
     * @param settings the settings associated with the input device
     * @param painter the painter used to draw
     */
    virtual KisPaintOp * createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageWSP image) = 0;
    virtual QString id() const = 0;
    virtual QString name() const = 0;

    /**
     * The filename of the pixmap we can use to represent this paintop in the ui.
     */
    virtual QString pixmap();

    /**
     * Whether this paintop is internal to a certain tool or can be used
     * in various tools. If false, it won't show up in the toolchest.
     * The KoColorSpace argument can be used when certain paintops only support a specific cs
     */
    virtual bool userVisible(const KoColorSpace * cs = 0);

    /**
     * Create and return an settings object for this paintop.
     */
    virtual KisPaintOpSettingsSP settings(KisImageWSP image) = 0;

    /**
     * create a widget that can display paintop settings
     */
    virtual KisPaintOpSettingsWidget* createSettingsWidget(QWidget* parent) = 0;

};

#endif
