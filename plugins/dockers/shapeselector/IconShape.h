/*
 * Copyright (C) 2006, 2008 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef ICONSHAPE_H
#define ICONSHAPE_H

#include <KoShape.h>

class KoCreateShapesTool;
class QDomElement;

class IconShape : public KoShape
{
public:
    IconShape(const QString &icon);
    virtual ~IconShape();

    /**
     * This method is called when the user selects this item to be created later.
     * The implementation should set all its options on the KoCreateShapesTool instance.
     */
    virtual void visit(KoCreateShapesTool *tool) = 0;
    /**
     * Return the tooltip that should be shown when the user hovers over this item.
     */
    virtual QString toolTip() = 0;
    virtual void save(QDomElement &root) = 0;
    /// reimplemented
    virtual void saveOdf( KoShapeSavingContext & ) const {}
    /// reimplemented
    virtual bool loadOdf( const KoXmlElement &, KoShapeLoadingContext &) { return true; }
    /// reimplemented
    virtual void paint(QPainter &painter, const KoViewConverter &converter);

    QPixmap pixmap() const { return m_icon; }

private:
    QPixmap m_icon;
};

#endif
