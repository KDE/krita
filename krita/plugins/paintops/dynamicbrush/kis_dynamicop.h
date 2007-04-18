/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_DYNAMICOP_H_
#define KIS_DYNAMICOP_H_

#include "kis_paintop.h"

class QWidget;
class QPointF;
class KisPainter;

class KisDynamicBrush;
class Ui_DynamicBrushOptions;

class KisDynamicOpFactory : public KisPaintOpFactory  {
    public:
        KisDynamicOpFactory() {}
        virtual ~KisDynamicOpFactory() {}
    
        virtual KisPaintOp * createOp(const KisPaintOpSettings *settings, KisPainter * painter);
        virtual QString id() const { return "dynamicbrush"; }
        virtual QString name()  const{ return i18n("Dynamic Brush"); }
        virtual QString pixmap() { return "dynamicbrush.png"; }
        virtual KisPaintOpSettings *settings(QWidget * parent, const KoInputDevice& inputDevice);
    private:
};

class KisDynamicOpSettings : public QObject, public KisPaintOpSettings {
    Q_OBJECT
    public:
        KisDynamicOpSettings(QWidget* parent);
        virtual ~KisDynamicOpSettings();
        virtual QWidget *widget() const { return m_optionsWidget; }
        /// @return a brush with the current shapes, coloring and program
        KisDynamicBrush* createBrush() const;
    private:
        QWidget* m_optionsWidget;
        Ui_DynamicBrushOptions* m_uiOptions;
};

class KisDynamicOp : public KisPaintOp {

    typedef KisPaintOp super;

public:

    KisDynamicOp(const KisDynamicOpSettings *settings, KisPainter * painter);
    virtual ~KisDynamicOp();

    void paintAt(const QPointF &pos, const KisPaintInformation& info);

private:
    KisDynamicBrush* m_brush;
    const KisDynamicOpSettings *m_settings;
};

#endif // KIS_DYNAMICOP_H_
