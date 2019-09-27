/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
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


#include "ColorSelectorItem.h"
#include <plugins/dockers/advancedcolorselector/kis_color_selector_component.h>
#include <plugins/dockers/advancedcolorselector/kis_color_selector_ring.h>
#include <plugins/dockers/advancedcolorselector/kis_color_selector_triangle.h>
#include <plugins/dockers/advancedcolorselector/kis_color_selector_simple.h>
#include <plugins/dockers/advancedcolorselector/kis_color_selector_wheel.h>
#include <kis_canvas2.h>
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>
#include <KoCanvasResourceProvider.h>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include "kis_display_color_converter.h"

#include <math.h>

class ColorSelectorItem::Private
{
public:
    Private(ColorSelectorItem* qq)
        : q(qq)
        , selector(new KisColorSelector)
        , view(0)
        , colorRole(Acs::Foreground)
        , grabbingComponent(0)
        , colorUpdateAllowed(true)
        , changeBackground(false)
        , shown(true)
        , repaintTimer(new QTimer)
    {
        ring = new KisColorSelectorRing(selector);
        ring->setInnerRingRadiusFraction(0.7);
        triangle = new KisColorSelectorTriangle(selector);
        slider = new KisColorSelectorSimple(selector);
        square = new KisColorSelectorSimple(selector);
        wheel = new KisColorSelectorWheel(selector);
        main = triangle;
        sub = ring;
        connect(main, SIGNAL(paramChanged(qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal)),
                sub,  SLOT(setParam(qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal)), Qt::UniqueConnection);
        connect(sub,  SIGNAL(paramChanged(qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal)),
                main, SLOT(setParam(qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal)), Qt::UniqueConnection);

        main->setConfiguration(selector->configuration().mainTypeParameter, selector->configuration().mainType);
        sub->setConfiguration(selector->configuration().subTypeParameter, selector->configuration().subType);

        repaintTimer->setInterval(50);
        repaintTimer->setSingleShot(true);
        connect(repaintTimer, SIGNAL(timeout()), q, SLOT(repaint()));
    }
    ~Private()
    {
        selector->deleteLater();
    }
    void repaint();
    QImage paintedItem;

    ColorSelectorItem* q;

    KisColorSelector* selector;

    KisColorSelectorRing* ring;
    KisColorSelectorTriangle* triangle;
    KisColorSelectorSimple* slider;
    KisColorSelectorSimple* square;
    KisColorSelectorWheel* wheel;

    KisColorSelectorComponent* main;
    KisColorSelectorComponent* sub;

    KisViewManager* view;
    Acs::ColorRole colorRole;
    KoColor currentColor;
    KisColorSelectorComponent* grabbingComponent;

    void commitColor(const KoColor& color, Acs::ColorRole role);
    bool colorUpdateAllowed;
    bool changeBackground;
    bool shown;
    QTimer* repaintTimer;

    void colorChangedImpl(const KoColor &color, Acs::ColorRole role);
};

void ColorSelectorItem::Private::commitColor(const KoColor& color, Acs::ColorRole role)
{
    if (!view->canvas())
        return;

    KoColor currentColor = Acs::currentColor(view->canvasResourceProvider(), role);
    if (color == currentColor) return;

    colorUpdateAllowed = false;
    Acs::setCurrentColor(view->canvasResourceProvider(), role, color);
    QColor qcolor = selector->converter()->toQColor(currentColor);
    emit q->colorChanged(qcolor, color.opacityF(), false);
    colorUpdateAllowed = true;
}

ColorSelectorItem::ColorSelectorItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
    , d(new Private(this))
{
    setFlag(QQuickItem::ItemHasContents, true);
    setAcceptedMouseButtons( Qt::LeftButton | Qt::RightButton );
}

ColorSelectorItem::~ColorSelectorItem()
{
    delete d;
}

void ColorSelectorItem::paint(QPainter* painter)
{
    if(!d->shown)
        return;
    painter->drawImage(boundingRect(), d->paintedItem);

}

void ColorSelectorItem::Private::repaint()
{
    paintedItem = QImage(q->boundingRect().size().toSize(), QImage::Format_ARGB32_Premultiplied);
    if(paintedItem.isNull())
        return;
    paintedItem.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&paintedItem);
    main->paintEvent(&painter);
    sub->paintEvent(&painter);
    painter.end();
    q->update();
}

void ColorSelectorItem::geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry)
{
//    QRectF bounds = boundingRect();
//    if (d->selector->configuration().subType==KisColorSelector::Ring)
//    {
//        d->ring->setGeometry(bounds.x(),bounds.y(),bounds.width(), bounds.height());
//        if (d->selector->configuration().mainType==KisColorSelector::Triangle)
//        {
//            d->triangle->setGeometry(bounds.width()/2 - d->ring->innerRadius(),
//                                     bounds.height()/2 - d->ring->innerRadius(),
//                                     d->ring->innerRadius()*2,
//                                     d->ring->innerRadius()*2);
//        }
//        else
//        {
//            int size = d->ring->innerRadius()*2/sqrt(2.);
//            d->square->setGeometry(bounds.width()/2 - size/2,
//                                   bounds.height()/2 - size/2,
//                                   size,
//                                   size);
//        }
//    }
//    else
//    {
//        // type wheel and square
//        if (d->selector->configuration().mainType==KisColorSelector::Wheel)
//        {
//            d->main->setGeometry(bounds.x(), bounds.y() + height()*0.1, bounds.width(), bounds.height()*0.9);
//            d->sub->setGeometry( bounds.x(), bounds.y(),                bounds.width(), bounds.height()*0.1);
//        }
//        else
//        {
//            if (bounds.height()>bounds.width())
//            {
//                d->main->setGeometry(bounds.x(), bounds.y() + bounds.height()*0.1, bounds.width(), bounds.height()*0.9);
//                d->sub->setGeometry( bounds.x(), bounds.y(),                       bounds.width(), bounds.height()*0.1);
//            }
//            else
//            {
//                d->main->setGeometry(bounds.x(), bounds.y() + bounds.height()*0.1, bounds.width(), bounds.height()*0.9);
//                d->sub->setGeometry( bounds.x(), bounds.y(),                       bounds.width(), bounds.height()*0.1);
//            }
//        }
//    }

    if (d->view) {
        d->selector->setColor(Acs::currentColor(d->view->canvasResourceProvider(), d->colorRole));
    }

    d->repaintTimer->start();
    QQuickPaintedItem::geometryChanged(newGeometry, oldGeometry);
}

void ColorSelectorItem::mousePressEvent(QMouseEvent* event)
{
    d->colorRole = d->changeBackground ?
        Acs::Background : Acs::buttonToRole(event->button());

    if (d->main->wantsGrab(event->pos().x(), event->pos().y())) {
        d->grabbingComponent = d->main;
    } else if (d->sub->wantsGrab(event->pos().x(), event->pos().y())) {
        d->grabbingComponent = d->sub;
    }

    mouseEvent(event);
}

void ColorSelectorItem::mouseMoveEvent(QMouseEvent* event)
{
    mouseEvent(event);
}

void ColorSelectorItem::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    d->grabbingComponent=0;
}

void ColorSelectorItem::mouseEvent(QMouseEvent* event)
{
    if (d->grabbingComponent && (event->buttons()&Qt::LeftButton || event->buttons()&Qt::RightButton))
    {
        d->grabbingComponent->mouseEvent(event->pos().x(), event->pos().y());

        qreal alpha = d->currentColor.opacityF();
        d->currentColor = d->main->currentColor();
        d->currentColor.setOpacity(alpha);
        d->commitColor(d->currentColor, d->colorRole);
        d->repaintTimer->start();
    }
}

QObject* ColorSelectorItem::view() const
{
    return d->view;
}

void ColorSelectorItem::setView(QObject* newView)
{
    d->view = qobject_cast<KisViewManager*>( newView );
    if (d->view) {
        connect(d->view->canvasResourceProvider(), SIGNAL(sigFGColorChanged(KoColor)),
                this, SLOT(fgColorChanged(KoColor)));
        connect(d->view->canvasResourceProvider(), SIGNAL(sigBGColorChanged(KoColor)),
                this, SLOT(bgColorChanged(KoColor)));

        d->commitColor(d->currentColor, d->colorRole);
        setChangeBackground(changeBackground());
    }
    emit viewChanged();
}

bool ColorSelectorItem::changeBackground() const
{
    return d->changeBackground;
}

void ColorSelectorItem::setChangeBackground(bool newChangeBackground)
{
    d->changeBackground = newChangeBackground;
    d->colorRole = newChangeBackground ? Acs::Background : Acs::Foreground;
    emit changeBackgroundChanged();
    if (!d->view)
        return;


    d->currentColor = Acs::currentColor(d->view->canvasResourceProvider(), d->colorRole);

    d->main->setColor(d->currentColor);
    d->sub->setColor(d->currentColor);
    d->repaintTimer->start();
}

bool ColorSelectorItem::shown() const
{
    return d->shown;
}

void ColorSelectorItem::setShown(bool newShown)
{
    d->shown = newShown;
    emit shownChanged();
}

void ColorSelectorItem::setAlpha(int percentValue)
{
    qreal alpha = (float)percentValue / 100.0;
    d->currentColor.setOpacity(alpha);
    if (d->view) {
        d->commitColor(d->currentColor, d->colorRole);
    }
}

void ColorSelectorItem::Private::colorChangedImpl(const KoColor &newColor, Acs::ColorRole role)
{
    if (colorRole != role) return;
    if (colorUpdateAllowed == false) return;

    if(newColor == currentColor) return;

    currentColor = newColor;
    main->setColor(newColor);
    sub->setColor(newColor);
    commitColor(currentColor, colorRole);
    QColor qcolor = selector->converter()->toQColor(currentColor);
    emit q->colorChanged(qcolor, currentColor.opacityF(), false);
    repaintTimer->start();
}

void ColorSelectorItem::fgColorChanged(const KoColor& newColor)
{
    d->colorChangedImpl(newColor, Acs::Foreground);
}

void ColorSelectorItem::bgColorChanged(const KoColor& newColor)
{
    d->colorChangedImpl(newColor, Acs::Background);
}

void ColorSelectorItem::repaint()
{
    d->repaint();
}

