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
#include <KoReportItemBase.h>
#include <QRect>
#include <QPainter>
#include <QDomDocument>
#include <krpos.h>
#include <krsize.h>


namespace Scripting
{
class Shape;
}
/**
 @author
*/
class KoReportItemShape : public KoReportItemBase
{
public:
    KoReportItemShape() {
        createProperties();
    };
    KoReportItemShape(QDomNode & element);
    ~KoReportItemShape() {};

    virtual QString typeName() const;
    using KoReportItemBase::render;
    virtual int render(OROPage* page, OROSection* section,  QPointF offset, QVariant data, KRScriptHandler *script);

protected:
    QRectF _rect();

    KRSize m_size;
    KoProperty::Property *m_shapeType;

private:
    virtual void createProperties();

    friend class Scripting::Shape;
};
#endif
