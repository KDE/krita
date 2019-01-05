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

#include <QScopedPointer>

#include <compositeops/KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
#include <KoID.h>
#include <klocalizedstring.h>

#include "kritaimage_export.h"

class QDomElement;
class QDomDocument;
class KisBrushMaskApplicatorBase;

const KoID DefaultId("default", ki18n("Default")); ///< generate Krita default mask generator
const KoID SoftId("soft", ki18n("Soft")); ///< generate brush mask from former softbrush paintop, where softness is based on curve
const KoID GaussId("gauss", ki18n("Gaussian")); ///< generate brush mask with a Gaussian-blurred edge

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
     * This function creates an auto brush shape with the following values:
     * @param radius radius
     * @param ratio aspect ratio
     * @param fh horizontal fade
     * @param fv vertical fade
     * @param spikes number of spikes
     * @param antialiasEdges whether to antialias edges
     * @param id the brush identifier
     */
    KisMaskGenerator(qreal radius, qreal ratio, qreal fh, qreal fv, int spikes, bool antialiasEdges, Type type, const KoID& id = DefaultId);
    KisMaskGenerator(const KisMaskGenerator &rhs);

    virtual ~KisMaskGenerator();

    virtual KisMaskGenerator* clone() const = 0;

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
    void setDiameter(qreal value);

    qreal ratio() const;
    qreal horizontalFade() const;
    qreal verticalFade() const;
    int spikes() const;
    Type type() const;
    bool isEmpty() const;
    void fixRotation(qreal &xr, qreal &yr) const;
    
    inline QString id() const { return m_id.id(); }
    inline QString name() const { return m_id.name(); }

    static QList<KoID> maskGeneratorIds();
    
    qreal softness() const;
    virtual void setSoftness(qreal softness);
    
    QString curveString() const;
    void setCurveString(const QString& curveString);

    bool antialiasEdges() const;
    virtual void setScale(qreal scaleX, qreal scaleY);

protected:
    qreal effectiveSrcWidth() const;
    qreal effectiveSrcHeight() const;

private:
    struct Private;
    const QScopedPointer<Private> d;
    const KoID& m_id;
};

#endif
