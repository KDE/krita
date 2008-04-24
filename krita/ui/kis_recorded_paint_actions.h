/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_RECORDED_PAINT_ACTIONS_H_
#define _KIS_RECORDED_PAINT_ACTIONS_H_

#include "kis_recorded_action.h"
#include "kis_paintop_settings.h"
#include "kis_types.h"

class KisPaintInformation;
class KisBrush;
class KisPainter;
class KoColor;
class KoCompositeOp;

#include <krita_export.h>

class KRITAUI_EXPORT KisRecordedPaintAction : public KisRecordedAction {
    public:

        KisRecordedPaintAction(const QString & name, const QString & id, KisLayerSP layer, KisBrush* brush, const QString & paintOpId, const KisPaintOpSettingsSP settings, KoColor foregroundColor, KoColor backgroundColor, int opacity, bool paintIncremental, const KoCompositeOp * compositeOp);
        
        KisRecordedPaintAction(const KisRecordedPaintAction&);

        ~KisRecordedPaintAction();

        virtual void toXML(QDomDocument& doc, QDomElement& elt) const;

        virtual void play(KisUndoAdapter* adapter = 0) const;
        
    protected:

        virtual void playPaint(KisPainter* painter) const = 0;

        KisLayerSP layer() const;

        KisBrush* brush() const;

        QString paintOpId() const;
        
    private:

        struct Private;
        Private* const d;
};

class KRITAUI_EXPORT KisRecordedPolyLinePaintAction : public KisRecordedPaintAction {

    public:
        
        KisRecordedPolyLinePaintAction(const QString & name, KisLayerSP layer, KisBrush* brush, const QString & paintOpId, const

        KisPaintOpSettingsSP settings, KoColor foregroundColor, KoColor backgroundColor, int opacity, bool paintIncremental, const KoCompositeOp * compositeOp);

        KisRecordedPolyLinePaintAction(const KisRecordedPolyLinePaintAction&);

        ~KisRecordedPolyLinePaintAction();

        void addPoint(const KisPaintInformation& info);

        virtual void toXML(QDomDocument& doc, QDomElement& elt) const;

        virtual KisRecordedAction* clone() const;
        
    protected:
    
        virtual void playPaint(KisPainter* painter) const;
        
    private:
    
        struct Private;
        Private* const d;
};

class KRITAUI_EXPORT KisRecordedBezierCurvePaintAction : public KisRecordedPaintAction {

    public:
    
        KisRecordedBezierCurvePaintAction(const QString & name, KisLayerSP layer, KisBrush* brush, const QString & paintOpId, const KisPaintOpSettingsSP settings, KoColor foregroundColor, KoColor backgroundColor, int opacity, bool paintIncremental, const KoCompositeOp * compositeOp);
        
        KisRecordedBezierCurvePaintAction(const KisRecordedBezierCurvePaintAction&);

        ~KisRecordedBezierCurvePaintAction();

        void addPoint(const KisPaintInformation& point1, const QPointF& control1, const QPointF& control2, const KisPaintInformation& point2);

        virtual void toXML(QDomDocument& doc, QDomElement& elt) const;

        virtual KisRecordedAction* clone() const;
        
    protected:
    
        virtual void playPaint(KisPainter* painter) const;
        
    private:
    
        struct Private;
        Private* const d;
};


#endif
