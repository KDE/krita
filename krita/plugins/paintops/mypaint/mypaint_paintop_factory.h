/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_MYPAINT_PAINTOP_FACTORY_H_
#define KIS_MYPAINT_PAINTOP_FACTORY_H_

#include <QString>

#include <klocale.h>

#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

#include <kis_paintop_factory.h>
#include <kis_types.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

class KisPaintOp;
class KisPainter;
class QWidget;
class KoInputDevice;
class MyPaintSettingsWidget;
class MyPaintSettings;
class MyPaintBrushResource;

class MyPaintFactory : public KisPaintOpFactory
{

    Q_OBJECT

public:
    MyPaintFactory();
    virtual ~MyPaintFactory();

    virtual KisPaintOp * createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageWSP image);

    virtual QString id() const {
        return "mypaintbrush";
    }

    virtual QString name() const {
        return i18n("MyPaint brush");
    }

    virtual QString pixmap() {
        return "krita-mypaint.png";
    }

    virtual KisPaintOpSettingsSP settings(KisImageWSP image);
    virtual KisPaintOpSettingsWidget* createSettingsWidget(QWidget* parent);

    QList<MyPaintBrushResource*> brushes() const;
    MyPaintBrushResource* brush(const QString& fileName) const;

private slots:

    void brushThreadDone();

private:

    class Private;
    Private* const m_d;

};

#endif

