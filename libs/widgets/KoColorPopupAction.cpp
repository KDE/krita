/* This file is part of the KDE project
 * Copyright (c) 2007 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
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

#include "KoColorPopupAction.h"

#include "KoColorSetWidget.h"
#include "KoTriangleColorSelector.h"
#include "KoColorSlider.h"
#include "KoCheckerBoardPainter.h"
#include "KoResourceServer.h"
#include "KoResourceServerProvider.h"
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>

#include <WidgetsDebug.h>
#include <klocalizedstring.h>

#include <QPainter>
#include <QWidgetAction>
#include <QMenu>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QToolButton>


class KoColorPopupAction::KoColorPopupActionPrivate
{
public:
    KoColorPopupActionPrivate()
        : colorSetWidget(0), colorChooser(0), opacitySlider(0), menu(0), checkerPainter(4)
        , showFilter(true), applyMode(true), firstTime(true)
    {}

    ~KoColorPopupActionPrivate()
    {
        delete colorSetWidget;
        delete colorChooser;
        delete opacitySlider;
        delete menu;
    }

    KoColor currentColor;
    KoColor buddyColor;

    KoColorSetWidget *colorSetWidget;
    KoTriangleColorSelector * colorChooser;
    KoColorSlider * opacitySlider;
    QMenu *menu;
    KoCheckerBoardPainter checkerPainter;
    bool showFilter;
    bool applyMode;

    bool firstTime;
};

KoColorPopupAction::KoColorPopupAction(QObject *parent)
    : QAction(parent),
    d(new KoColorPopupActionPrivate())
{
    d->menu = new QMenu();
    QWidget *widget = new QWidget(d->menu);
    QWidgetAction *wdgAction = new QWidgetAction(d->menu);
    d->colorSetWidget = new KoColorSetWidget(widget);

    d->colorChooser = new KoTriangleColorSelector( widget );
    // prevent mouse release on color selector from closing popup
    d->colorChooser->setAttribute( Qt::WA_NoMousePropagation );
    d->opacitySlider = new KoColorSlider( Qt::Vertical, widget );
    d->opacitySlider->setFixedWidth(25);
    d->opacitySlider->setRange(0, 255);
    d->opacitySlider->setValue(255);
    d->opacitySlider->setToolTip( i18n( "Opacity" ) );

    QGridLayout * layout = new QGridLayout( widget );
    layout->addWidget( d->colorSetWidget, 0, 0, 1, -1 );
    layout->addWidget( d->colorChooser, 1, 0 );
    layout->addWidget( d->opacitySlider, 1, 1 );
    layout->setMargin(4);

    wdgAction->setDefaultWidget(widget);
    d->menu->addAction(wdgAction);
    setMenu(d->menu);
    new QHBoxLayout(d->menu);
    d->menu->layout()->addWidget(widget);
    d->menu->layout()->setMargin(0);

    connect(this, SIGNAL(triggered()), this, SLOT(emitColorChanged()));

    connect(d->colorSetWidget, SIGNAL(colorChanged(const KoColor &, bool)), this, SLOT(colorWasSelected(const KoColor &, bool)));

    connect( d->colorChooser, SIGNAL( colorChanged( const QColor &) ),
             this, SLOT( colorWasEdited( const QColor &) ) );
    connect( d->opacitySlider, SIGNAL(valueChanged(int)),
             this, SLOT(opacityWasChanged(int)));
}

KoColorPopupAction::~KoColorPopupAction()
{
    delete d;
}

void KoColorPopupAction::setCurrentColor( const KoColor &color )
{
    KoColor minColor( color );
    d->currentColor = minColor;

    d->colorChooser->blockSignals(true);
    d->colorChooser->slotSetColor(color);
    d->colorChooser->blockSignals(false);

    KoColor maxColor( color );
    minColor.setOpacity( OPACITY_TRANSPARENT_U8 );
    maxColor.setOpacity( OPACITY_OPAQUE_U8 );

    d->opacitySlider->blockSignals( true );
    d->opacitySlider->setColors( minColor, maxColor );
    d->opacitySlider->setValue( color.opacityU8() );
    d->opacitySlider->blockSignals( false );

    updateIcon();
}

void KoColorPopupAction::setCurrentColor( const QColor &_color )
{
#ifndef NDEBUG
    if (!_color.isValid()) {
        warnWidgets << "Invalid color given, defaulting to black";
    }
#endif
    const QColor color(_color.isValid() ? _color : QColor(0,0,0,255));
    setCurrentColor(KoColor(color, KoColorSpaceRegistry::instance()->rgb8() ));
}

QColor KoColorPopupAction::currentColor() const
{
    return d->currentColor.toQColor();
}

KoColor KoColorPopupAction::currentKoColor() const
{
    return d->currentColor;
}

void KoColorPopupAction::updateIcon()
{
    QSize iconSize;
    QToolButton *toolButton = dynamic_cast<QToolButton*>(parentWidget());
    if (toolButton) {
        iconSize = QSize(toolButton->iconSize());
    } else {
        iconSize = QSize(16, 16);
    }

    // This must be a QImage, as drawing to a QPixmap outside the
    // UI thread will cause sporadic crashes.
    QImage pm;

    if (icon().isNull()) {
        d->applyMode = false;
    }

    if(d->applyMode) {
        pm = icon().pixmap(iconSize).toImage();
        if (pm.isNull()) {
            pm = QImage(iconSize, QImage::Format_ARGB32_Premultiplied);
            pm.fill(Qt::transparent);
        }
        QPainter p(&pm);
        p.fillRect(0, iconSize.height() - 4, iconSize.width(), 4, d->currentColor.toQColor());
        p.end();
    } else {
        pm = QImage(iconSize, QImage::Format_ARGB32_Premultiplied);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        d->checkerPainter.paint(p, QRect(QPoint(),iconSize));
        p.fillRect(0, 0, iconSize.width(), iconSize.height(), d->currentColor.toQColor());
        p.end();
    }
    setIcon(QIcon(QPixmap::fromImage(pm)));
}

void KoColorPopupAction::emitColorChanged()
{
    emit colorChanged( d->currentColor );
}

void KoColorPopupAction::colorWasSelected(const KoColor &color, bool final)
{
    d->currentColor = color;
    if (final) {
        menu()->hide();
        emitColorChanged();
    }
    updateIcon();
}

void KoColorPopupAction::colorWasEdited( const QColor &color )
{
    d->currentColor = KoColor( color, KoColorSpaceRegistry::instance()->rgb8() );
    quint8 opacity = d->opacitySlider->value();
    d->currentColor.setOpacity( opacity );

    KoColor minColor = d->currentColor;
    minColor.setOpacity( OPACITY_TRANSPARENT_U8 );
    KoColor maxColor = minColor;
    maxColor.setOpacity( OPACITY_OPAQUE_U8 );

    d->opacitySlider->setColors( minColor, maxColor );

    emitColorChanged();

    updateIcon();
}

void KoColorPopupAction::opacityWasChanged( int opacity )
{
    d->currentColor.setOpacity( quint8(opacity) );

    emitColorChanged();
}

void KoColorPopupAction::slotTriggered(bool)
{
    if (d->firstTime) {
        KoResourceServer<KoColorSet>* srv = KoResourceServerProvider::instance()->paletteServer(false);
        QList<KoColorSet*> palettes = srv->resources();
        if (!palettes.empty()) {
            d->colorSetWidget->setColorSet(palettes.first());
        }
        d->firstTime = false;
    }
}
