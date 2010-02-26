/*
 * Kexi Report Plugin
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KRSHAPEDATA_H
#define KRSHAPEDATA_H
#include "krobjectdata.h"
#include <QRect>
#include <QPainter>
#include <qdom.h>
#include "krpos.h"
#include "krsize.h"


namespace Scripting
{
class Shape;
}
/**
 @author
*/
class KRShapeData : public KRObjectData
{
public:
    KRShapeData() {
        createProperties();
    };
    KRShapeData(QDomNode & element);
    ~KRShapeData() {};
    virtual int type() const;
    virtual KRShapeData * toShape();

protected:
    QRectF _rect();

    KRSize m_size;
    KoProperty::Property *m_shapeType;

private:
    virtual void createProperties();
    static int RTTI;

    friend class Scripting::Shape;
    friend class KoReportPreRendererPrivate;
};
#endif
