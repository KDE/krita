/* Part of Calligra Suite - Map Shape
   Copyright 2007 Montel Laurent <montel@kde.org>
   Copyright 2008 Simon Schmeisser <mail_to_wrt@gmx.de>
   Copyright (C) 2011  Radosław Wicik <radoslaw@wicik.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef GEO_TOOL
#define GEO_TOOL

#include <KoToolBase.h>

#include <global.h>


class MapShape;
namespace Marble  {
    class MarbleControlBox;
    class MapThemeManager;
    class GeoDataLatLonAltBox;
}

class QModelIndex;
class MapToolPrivate;

class MapTool : public KoToolBase
{
    Q_OBJECT
    typedef Marble::Projection Projection;
public:
    explicit MapTool(KoCanvasBase* canvas);
    virtual ~MapTool();

    /// reimplemented from KoToolBase
    virtual void paint(QPainter& , const KoViewConverter&) {}
    /// reimplemented from KoToolBase
    virtual void mousePressEvent(KoPointerEvent*) {}
    /// reimplemented from superclass
    virtual void mouseDoubleClickEvent(KoPointerEvent *event);
    /// reimplemented from KoToolBase
    virtual void mouseMoveEvent(KoPointerEvent*) {}
    /// reimplemented from KoToolBase
    virtual void mouseReleaseEvent(KoPointerEvent*) {}

    /// reimplemented from KoToolBase
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    /// reimplemented from KoToolBase
    virtual void deactivate();

protected:
    /// reimplemented from KoToolBase
    virtual QWidget * createOptionWidget();

public Q_SLOTS:
    void mapContentChanged(Marble::GeoDataLatLonAltBox& geoData);

    void setProjection(Projection);
    void setMapThemeId(const QString&);
    //void centerOn(const QModelIndex&);
    void zoomChanged(int zoom);

private:
    MapToolPrivate *d;
};

#endif
