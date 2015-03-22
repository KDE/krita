/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_TRANSFORM_MASK_PARAMS_INTERFACE_H
#define __KIS_TRANSFORM_MASK_PARAMS_INTERFACE_H

#include "krita_export.h"
#include "kis_types.h"

#include <QScopedPointer>


class QTransform;
class QDomElement;

class KRITAIMAGE_EXPORT KisTransformMaskParamsInterface
{
public:
    virtual ~KisTransformMaskParamsInterface();

    virtual QTransform finalAffineTransform() const = 0;
    virtual bool isAffine() const = 0;
    virtual bool isHidden() const = 0;

    virtual void transformDevice(KisNodeSP node, KisPaintDeviceSP src, KisPaintDeviceSP dst) const = 0;

    virtual QString id() const = 0;
    virtual void toXML(QDomElement *e) const = 0;

    virtual void translate(const QPointF &offset) = 0;
};


class QDomElement;

class KRITAIMAGE_EXPORT KisDumbTransformMaskParams : public KisTransformMaskParamsInterface
{
public:
    KisDumbTransformMaskParams();
    KisDumbTransformMaskParams(const QTransform &transform);
    KisDumbTransformMaskParams(bool isHidden);
    ~KisDumbTransformMaskParams();


    QTransform finalAffineTransform() const;
    bool isAffine() const;
    bool isHidden() const;
    void transformDevice(KisNodeSP node, KisPaintDeviceSP src, KisPaintDeviceSP dst) const;

    QString id() const;
    void toXML(QDomElement *e) const;
    static KisTransformMaskParamsInterfaceSP fromXML(const QDomElement &e);

    void translate(const QPointF &offset);

    // for tesing purposes only
    QTransform testingGetTransform() const;
    void testingSetTransform(const QTransform &t);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_TRANSFORM_MASK_PARAMS_INTERFACE_H */
