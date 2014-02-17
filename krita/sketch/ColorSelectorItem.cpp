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
#include <plugins/extensions/dockers/advancedcolorselector/kis_color_selector_component.h>
#include <plugins/extensions/dockers/advancedcolorselector/kis_color_selector_ring.h>
#include <plugins/extensions/dockers/advancedcolorselector/kis_color_selector_triangle.h>
#include <plugins/extensions/dockers/advancedcolorselector/kis_color_selector_simple.h>
#include <plugins/extensions/dockers/advancedcolorselector/kis_color_selector_wheel.h>
#include <kis_canvas2.h>
#include <kis_view2.h>
#include <kis_canvas_resource_provider.h>
#include <KoCanvasResourceManager.h>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

class ColorSelectorItem::Private
{
public:
    Private(ColorSelectorItem* qq)
        : q(qq)
        , selector(new KisColorSelector)
        , view(0)
        , colorRole(KisColorSelectorBase::Foreground)
        , currentColor(0, 0, 0)
        , grabbingComponent(0)
        , colorUpdateAllowed(true)
        , changeBackground(false)
        , shown(true)
    {
        ring = new KisColorSelectorRing(selector);
        triangle = new KisColorSelectorTriangle(selector);
        slider = new KisColorSelectorSimple(selector);
        square = new KisColorSelectorSimple(selector);
        wheel = new KisColorSelectorWheel(selector);
        main = triangle;
        sub = ring;
        connect(main, SIGNAL(paramChanged(qreal,qreal,qreal,qreal,qreal)),
                sub,  SLOT(setParam(qreal,qreal,qreal,qreal,qreal)), Qt::UniqueConnection);
        connect(sub,  SIGNAL(paramChanged(qreal,qreal,qreal,qreal,qreal)),
                main, SLOT(setParam(qreal,qreal,qreal,qreal, qreal)), Qt::UniqueConnection);

        main->setConfiguration(selector->configuration().mainTypeParameter, selector->configuration().mainType);
        sub->setConfiguration(selector->configuration().subTypeParameter, selector->configuration().subType);
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

    KisView2* view;
    KisColorSelectorBase::ColorRole colorRole;
    QColor currentColor;
    KisColorSelectorComponent* grabbingComponent;

    void commitColor(const KoColor& color, KisColorSelectorBase::ColorRole role);
    bool colorUpdateAllowed;
    bool changeBackground;
    bool shown;
};

void ColorSelectorItem::Private::commitColor(const KoColor& color, KisColorSelectorBase::ColorRole role)
{
    if (!view->canvas())
        return;

    if (role==KisColorSelectorBase::Foreground)
    {
        if (view->resourceProvider()->fgColor() == color)
            return;
        colorUpdateAllowed=false;
        view->resourceProvider()->setFGColor(color);
        emit q->colorChanged(color.toQColor(), color.toQColor().alphaF(), false);
        colorUpdateAllowed=true;
    }
    else
    {
        if (view->resourceProvider()->bgColor() == color)
            return;
        colorUpdateAllowed=false;
        view->resourceProvider()->setBGColor(color);
        emit q->colorChanged(color.toQColor(), color.toQColor().alphaF(), true);
        colorUpdateAllowed=true;
    }
}

ColorSelectorItem::ColorSelectorItem(QDeclarativeItem* parent)
    : QDeclarativeItem(parent)
    , d(new Private(this))
{
    setFlag( QGraphicsItem::ItemHasNoContents, false );
    setAcceptedMouseButtons( Qt::LeftButton | Qt::RightButton );
}

ColorSelectorItem::~ColorSelectorItem()
{
    delete d;
}

void ColorSelectorItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if(!d->shown)
        return;
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->drawImage(boundingRect(), d->paintedItem);

}

void ColorSelectorItem::Private::repaint()
{
    paintedItem = QImage(q->boundingRect().size().toSize(), QImage::Format_ARGB32_Premultiplied);
    paintedItem.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&paintedItem);
    main->paintEvent(&painter);
    sub->paintEvent(&painter);
    painter.end();
}

void ColorSelectorItem::geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QRectF bounds = boundingRect();
    if (d->selector->configuration().subType==KisColorSelector::Ring)
    {
        d->ring->setGeometry(bounds.x(),bounds.y(),bounds.width(), bounds.height());
        if (d->selector->configuration().mainType==KisColorSelector::Triangle)
        {
            d->triangle->setGeometry(bounds.width()/2 - d->ring->innerRadius(),
                                     bounds.height()/2 - d->ring->innerRadius(),
                                     d->ring->innerRadius()*2,
                                     d->ring->innerRadius()*2);
        }
        else
        {
            int size = d->ring->innerRadius()*2/sqrt(2.);
            d->square->setGeometry(bounds.width()/2 - size/2,
                                   bounds.height()/2 - size/2,
                                   size,
                                   size);
        }
    }
    else
    {
        // type wheel and square
        if (d->selector->configuration().mainType==KisColorSelector::Wheel)
        {
            d->main->setGeometry(bounds.x(), bounds.y() + height()*0.1, bounds.width(), bounds.height()*0.9);
            d->sub->setGeometry( bounds.x(), bounds.y(),                bounds.width(), bounds.height()*0.1);
        }
        else
        {
            if (bounds.height()>bounds.width())
            {
                d->main->setGeometry(bounds.x(), bounds.y() + bounds.height()*0.1, bounds.width(), bounds.height()*0.9);
                d->sub->setGeometry( bounds.x(), bounds.y(),                       bounds.width(), bounds.height()*0.1);
            }
            else
            {
                d->main->setGeometry(bounds.x(), bounds.y() + bounds.height()*0.1, bounds.width(), bounds.height()*0.9);
                d->sub->setGeometry( bounds.x(), bounds.y(),                       bounds.width(), bounds.height()*0.1);
            }
        }
    }
    if (d->view)
    {
        if (d->colorRole == KisColorSelectorBase::Foreground)
            d->selector->setColor(d->view->resourceProvider()->resourceManager()->foregroundColor().toQColor());
        else
            d->selector->setColor(d->view->resourceProvider()->resourceManager()->backgroundColor().toQColor());
    }
    d->repaint();
    QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
}

void ColorSelectorItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && d->changeBackground != true)
        d->colorRole=KisColorSelectorBase::Foreground;
    else
        d->colorRole=KisColorSelectorBase::Background;
    if (d->main->wantsGrab(event->pos().x(), event->pos().y()))
        d->grabbingComponent=d->main;
    else if (d->sub->wantsGrab(event->pos().x(), event->pos().y()))
        d->grabbingComponent=d->sub;
    mouseEvent(event);
}

void ColorSelectorItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    mouseEvent(event);
}

void ColorSelectorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* /*event*/)
{
    d->grabbingComponent=0;
}

void ColorSelectorItem::mouseEvent(QGraphicsSceneMouseEvent* event)
{
    if (d->grabbingComponent && (event->buttons()&Qt::LeftButton || event->buttons()&Qt::RightButton))
    {
        d->grabbingComponent->mouseEvent(event->pos().x(), event->pos().y());

        qreal alpha = d->currentColor.alphaF();
        d->currentColor=d->main->currentColor();
        d->currentColor.setAlphaF(alpha);
        d->commitColor(KoColor(d->currentColor, d->view->resourceProvider()->fgColor().colorSpace()), d->colorRole);
        d->repaint();
        update();
    }
}

QObject* ColorSelectorItem::view() const
{
    return d->view;
}

void ColorSelectorItem::setView(QObject* newView)
{
    d->view = qobject_cast<KisView2*>( newView );
    if (d->view) {
        connect(d->view->resourceProvider(), SIGNAL(sigFGColorChanged(KoColor)),
                this, SLOT(fgColorChanged(KoColor)));
        connect(d->view->resourceProvider(), SIGNAL(sigBGColorChanged(KoColor)),
                this, SLOT(bgColorChanged(KoColor)));

        d->commitColor(KoColor(d->currentColor, d->view->resourceProvider()->fgColor().colorSpace()), d->colorRole);
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
    d->colorRole = newChangeBackground ? KisColorSelectorBase::Background : KisColorSelectorBase::Foreground;
    emit changeBackgroundChanged();
    if (!d->view)
        return;
    if (newChangeBackground)
        d->currentColor = d->view->resourceProvider()->bgColor().toQColor();
    else
        d->currentColor = d->view->resourceProvider()->fgColor().toQColor();
    d->main->setColor(d->currentColor);
    d->sub->setColor(d->currentColor);
    update();
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
    d->currentColor.setAlphaF(alpha);
    if (d->view) {
        d->commitColor(KoColor(d->currentColor, d->view->resourceProvider()->fgColor().colorSpace()), d->colorRole);
    }
}

void ColorSelectorItem::fgColorChanged(const KoColor& newColor)
{
    if (d->colorRole == KisColorSelectorBase::Foreground )
    {
        QColor c = d->selector->findGeneratingColor(newColor);
        if (d->colorUpdateAllowed==false)
            return;
        d->currentColor = c;
        d->main->setColor(c);
        d->sub->setColor(c);
        d->commitColor(KoColor(d->currentColor, d->view->resourceProvider()->fgColor().colorSpace()), d->colorRole);
        emit colorChanged(d->currentColor, d->currentColor.alphaF(), false);
        update();
    }
}

void ColorSelectorItem::bgColorChanged(const KoColor& newColor)
{
    if (d->colorRole == KisColorSelectorBase::Background )
    {
        QColor c = d->selector->findGeneratingColor(newColor);
        if (d->colorUpdateAllowed==false)
            return;
        d->currentColor = c;
        d->main->setColor(c);
        d->sub->setColor(c);
        d->commitColor(KoColor(d->currentColor, d->view->resourceProvider()->fgColor().colorSpace()), d->colorRole);
        emit colorChanged(d->currentColor, d->currentColor.alphaF(), true);
        update();
    }
}

#include "ColorSelectorItem.moc"