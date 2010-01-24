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

#ifndef _KIS_RECORDED_PAINT_ACTION_H_
#define _KIS_RECORDED_PAINT_ACTION_H_

#include "recorder/kis_recorded_action.h"
#include "kis_types.h"

class KisPaintInformation;
class KisPainter;
class KoColor;
class KoCompositeOp;

#include <krita_export.h>

/**
 * Base class for paint action.
 */
class KRITAIMAGE_EXPORT KisRecordedPaintAction : public KisRecordedAction
{
public:

    KisRecordedPaintAction(const QString & id,
                           const QString & name,
                           const KisNodeQueryPath& path,
                           const KisPaintOpPresetSP paintOpPreset,
                           KoColor foregroundColor,
                           KoColor backgroundColor,
                           int opacity,
                           bool paintIncremental,
                           const QString& compositeOp);

    KisRecordedPaintAction(const KisRecordedPaintAction&);

    ~KisRecordedPaintAction();

    virtual void toXML(QDomDocument& doc, QDomElement& elt) const;

    virtual void play(KisNodeSP node, const KisPlayInfo&) const;

protected:

    virtual void playPaint(const KisPlayInfo&, KisPainter* painter) const = 0;

public:
    KisPaintOpPresetSP paintOpPreset() const;
    void setPaintOpPreset(KisPaintOpPresetSP preset);
    int opacity() const;
    void setOpacity(int );
    KoColor paintColor() const;
    void setPaintColor(const KoColor& color);
    KoColor backgroundColor() const;
    void setBackgroundColor(const KoColor& color);
private:

    struct Private;
    Private* const d;
};

class KisRecordedPaintActionFactory : public KisRecordedActionFactory
{
public:
    KisRecordedPaintActionFactory(const QString & id) : KisRecordedActionFactory(id) {}
    virtual ~KisRecordedPaintActionFactory() {}
protected:

    KisPaintOpPresetSP paintOpPresetFromXML(const QDomElement& elt);
};


#endif
