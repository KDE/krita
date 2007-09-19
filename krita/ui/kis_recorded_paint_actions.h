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

#include "kis_types.h"

class KisPaintInformation;
class KisBrush;

#include <krita_export.h>

class KisRecordedPaintAction : public KisRecordedAction {
    public:
        KisRecordedPaintAction(QString name, QString id);
        KisRecordedPaintAction(const KisRecordedPaintAction&);
};

class KRITAUI_EXPORT KisRecordedPolyLinePaintAction : public KisRecordedPaintAction {
    public:
        KisRecordedPolyLinePaintAction(QString name, KisLayerSP layer, KisBrush* brush, QString paintOpId);
        KisRecordedPolyLinePaintAction(const KisRecordedPolyLinePaintAction&);
        ~KisRecordedPolyLinePaintAction();
        void addPoint(const KisPaintInformation& info);
        virtual void play();
        virtual void toXML(QDomDocument& doc, QDomElement& elt);
        virtual KisRecordedAction* clone() const;
    private:
        struct Private;
        Private* const d;
};

class KisRecordedPolyLinePaintActionFactory : public KisRecordedActionFactory {
    public:
        KisRecordedPolyLinePaintActionFactory();
        virtual ~KisRecordedPolyLinePaintActionFactory();
        virtual KisRecordedAction* fromXML(KisImageSP img, const QDomElement& elt);
    protected:
        KisBrush* brushFromXML(const QDomElement& elt); // <- probably need to be moved in a KisRecordedPaintActionFactory
};

#endif
