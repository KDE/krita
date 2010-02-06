/*
 *  Copyright (c) 2007,2010 Cyrille Berger <cberger@cberger.net>
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
#include "kis_painter.h"

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
                           KisPaintOpPresetSP paintOpPreset);

    KisRecordedPaintAction(const KisRecordedPaintAction&);

    ~KisRecordedPaintAction();

    virtual void toXML(QDomDocument& doc, QDomElement& elt, KisRecordedActionSaveContext* ) const;

    virtual void play(KisNodeSP node, const KisPlayInfo&) const;

protected:

    virtual void playPaint(const KisPlayInfo&, KisPainter* painter) const = 0;

public:
    KisPaintOpPresetSP paintOpPreset() const;
    void setPaintOpPreset(KisPaintOpPresetSP preset);
    /**
     * @return the opacity in the range 0.0->1.0
     */
    qreal opacity() const;
    void setOpacity(qreal );
    KoColor paintColor() const;
    void setPaintColor(const KoColor& color);
    KoColor backgroundColor() const;
    void setBackgroundColor(const KoColor& color);
    QString compositeOp();
    void setCompositeOp(const QString& );
    void setPaintIncremental(bool );
    void setStrokeStyle(KisPainter::StrokeStyle );
    void setFillStyle(KisPainter::FillStyle );
    void setPattern(const KisPattern* );
    void setGradient(const KoAbstractGradient* gradient);
    void setGenerator(const KisFilterConfiguration * generator);
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

    void setupPaintAction(KisRecordedPaintAction* action, const QDomElement& elt, const KisRecordedActionLoadContext*);
    KisPaintOpPresetSP paintOpPresetFromXML(const QDomElement& elt);
    KoColor paintColorFromXML(const QDomElement& elt);
    KoColor backgroundColorFromXML(const QDomElement& elt);
    KoColor colorFromXML(const QDomElement& elt, const QString& elementName);
    int opacityFromXML(const QDomElement& elt);
    bool paintIncrementalFromXML(const QDomElement& elt);
    QString compositeOpFromXML(const QDomElement& elt);
    KisNodeQueryPath nodeQueryPathFromXML(const QDomElement& elt);
};


#endif
