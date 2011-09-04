/*
    Part of Calligra Suite - Marble Map Shape
    Copyright (C) 2007 Thomas Zander <zander@kde.org>
    Copyright (C) 2008-2009 Simon Schmeißer <mail_to_wrt@gmx.de>
    Copyright (C) 2011  Radosław Wicik <radoslaw@wicik.pl>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef MARBLEMAPSHAPE_H
#define MARBLEMAPSHAPE_H

#include <KoShape.h>
#include <KoFrameShape.h>

#define MARBLEMAPSHAPEID "MarbleMapShape"

class MarbleMapShapePrivate;
class KoImageCollection;

namespace Marble{
    class MarbleWidget;
}

class MarbleMapShape :public QObject, public KoShape, public KoFrameShape
{
    Q_OBJECT
public:
    explicit MarbleMapShape();
    virtual ~MarbleMapShape();

    virtual void paint(QPainter& painter, const KoViewConverter& converter);
    virtual bool loadOdf(const KoXmlElement& element, KoShapeLoadingContext& context);
    virtual void saveOdf(KoShapeSavingContext& context) const;
    KoImageCollection *imageCollection() const;
    void setImageCollection(KoImageCollection *collection);
    Marble::MarbleWidget* marbleWidget()const;
    
public slots:
    void requestUpdate();
protected:
    virtual bool loadOdfFrameElement(const KoXmlElement& element, KoShapeLoadingContext& context);
private:
     MarbleMapShapePrivate * const d;
};

#endif // MARBLEMAPSHAPE_H
