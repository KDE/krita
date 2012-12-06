/*
 *  Copyright (c) 2008-2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef _KIS_MASK_GENERATOR_H_
#define _KIS_MASK_GENERATOR_H_

#include <KoID.h>
#include <klocale.h>

#include "krita_export.h"
#include "kis_brush_mask_applicator_base.h"

class QDomElement;
class QDomDocument;

const KoID DefaultId("default", ki18n("Default")); ///< generate Krita default mask generator
const KoID SoftId("soft", ki18n("Soft brush")); ///< generate brush mask from former softbrush paintop, where softness is based on curve
const KoID GaussId("gauss", ki18n("Gaussian brush")); ///< generate brush mask with a Gaussian-blurred edge

static const int OVERSAMPLING = 4;

/**
 * This is the base class for mask shapes
 * You should subclass it if you want to create a new
 * shape.
 */
class KRITAIMAGE_EXPORT KisMaskGenerator
{
public:
    enum Type {
        CIRCLE, RECTANGLE
    };
public:

    /**
     * This function creates an auto brush shape with the following value :
     * @param w width
     * @param h height
     * @param fh horizontal fade (fh \< w / 2 )
     * @param fv vertical fade (fv \< h / 2 )
     */
    KisMaskGenerator(qreal radius, qreal ratio, qreal fh, qreal fv, int spikes, Type type, const KoID& id = DefaultId);

    virtual ~KisMaskGenerator();

private:

    void init();

public:
    /**
     * @return the alpha value at the position (x,y)
     */
    virtual quint8 valueAt(qreal x, qreal y) const = 0;

    virtual bool shouldSupersample() const;

    virtual bool shouldVectorize() const;

    virtual KisBrushMaskApplicatorBase* applicator();

    virtual void toXML(QDomDocument& , QDomElement&) const;

    /**
     * Unserialise a \ref KisMaskGenerator
     */
    static KisMaskGenerator* fromXML(const QDomElement&);

    qreal width() const;

    qreal height() const;

    qreal diameter() const;
    qreal ratio() const;
    qreal horizontalFade() const;
    qreal verticalFade() const;
    int spikes() const;
    Type type() const;
    
    inline QString id() const { return m_id.id(); }
    inline QString name() const { return m_id.name(); }

    static QList<KoID> maskGeneratorIds();
    
    qreal softness() const;
    virtual void setSoftness(qreal softness);
    
    QString curveString() const;
    void setCurveString(const QString& curveString);
    
protected:
    struct Private {
        qreal diameter, ratio;
        qreal softness;
        qreal fh, fv;
        int spikes;
        qreal cachedSpikesAngle;
        qreal cs, ss;
        bool empty;
        Type type;
        QString curveString;
        KisBrushMaskApplicatorBase *defaultMaskProcessor;
    };

    Private* const d;
    
private:
    const KoID& m_id;
};

#endif
