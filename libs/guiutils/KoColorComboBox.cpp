
/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoColorComboBox.h"
#include "KoTriangleColorSelector.h"
#include "KoColorSlider.h"
#include "KoCheckerBoardPainter.h"
#include "KoColorSpaceRegistry.h"

#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QGridLayout>
#include <QtGui/QMouseEvent>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

class KoColorComboBox::Private
{
public:
    Private() 
        : colorChooser(0), opacitySlider(0), popup(0), checkerPainter(4)
    {}
    
    QColor currentColor()
    {
        QColor color = colorChooser->color();
        color.setAlpha( opacitySlider->value() );
        return color;
    }
    KoTriangleColorSelector * colorChooser;
    KoColorSlider * opacitySlider;
    QMenu * popup;
    KoCheckerBoardPainter checkerPainter;
};

KoColorComboBox::KoColorComboBox( QWidget * parent )
    : QComboBox(parent), d(new Private())
{
    d->popup = new QMenu( this );
    d->colorChooser = new KoTriangleColorSelector( d->popup );
    // prevent mouse release on color selector from closing popup
    d->colorChooser->setAttribute( Qt::WA_NoMousePropagation );
    d->opacitySlider = new KoColorSlider( Qt::Vertical, d->popup );
    d->opacitySlider->setFixedWidth(25);
    d->opacitySlider->setRange(0, 255);
    // TODO: readd when string freeze is over ?
    //d->opacitySlider->setToolTip( i18n( "Opacity" ) );
    QGridLayout * layout = new QGridLayout( d->popup );
    layout->addWidget( d->colorChooser, 0, 0 );
    layout->addWidget( d->opacitySlider, 0, 1 );

    connect( d->colorChooser, SIGNAL( colorChanged( const QColor &) ), 
             this, SLOT( colorHasChanged( const QColor &) ) );
    connect( d->opacitySlider, SIGNAL(valueChanged(int)), 
             this, SLOT(opacityHasChanged(int)));
}

KoColorComboBox::~KoColorComboBox()
{
    delete d;
}

void KoColorComboBox::setColor( const QColor &color )
{
    d->colorChooser->setQColor( color );

    KoColor minColor( color, KoColorSpaceRegistry::instance()->rgb8() );
    KoColor maxColor( color, KoColorSpaceRegistry::instance()->rgb8() );
    minColor.setOpacity( 0 );
    maxColor.setOpacity( 255 );
    d->opacitySlider->blockSignals( true );
    d->opacitySlider->setColors( minColor, maxColor );
    d->opacitySlider->setValue( color.alpha() );
    d->opacitySlider->blockSignals( false );

    update();
}

QColor KoColorComboBox::color() const
{
    return d->currentColor();
}

void KoColorComboBox::paintEvent( QPaintEvent * event )
{
    QComboBox::paintEvent( event );

    QStyleOptionComboBox option;
    option.initFrom( this );
    QRect r = style()->subControlRect( QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, this );

    r = r.adjusted( 2, 2, -2, -2 );

    QPainter painter( this );
    d->checkerPainter.paint( painter, r );
    painter.fillRect( r, QBrush( d->currentColor() ) );
}

void KoColorComboBox::mousePressEvent( QMouseEvent * event )
{
    QStyleOptionComboBox opt;
    opt.init( this );
    opt.subControls = QStyle::SC_All;
    opt.activeSubControls = QStyle::SC_ComboBoxArrow;
    QStyle::SubControl sc = style()->hitTestComplexControl(QStyle::CC_ComboBox, &opt,
                                                           mapFromGlobal(event->globalPos()),
                                                           this);
    // only clicking on combobox arrow shows popup,
    // otherwise the colorApplied signal is send with the current color
    if (sc == QStyle::SC_ComboBoxArrow)
        QComboBox::mousePressEvent( event );
    else
        emit colorApplied( d->currentColor() );
}

void KoColorComboBox::showPopup()
{
    static bool firstShowOfPopup = true;
    if(firstShowOfPopup) {
        //show popup a bit early so it can be layout'ed
        d->popup->show(); 
        firstShowOfPopup = false;
    }

    // default popup position is below the combobox
    QPoint popupPos = mapToGlobal( QPoint( width() - d->popup->width(), height() ) );

    // Make sure the popup is not drawn outside the screen area
    QRect screenRect = QApplication::desktop()->availableGeometry( d->popup );
    // if lower right corner is not on screen anymore, popup above combobox
    if( ! screenRect.contains( popupPos + QPoint( d->popup->width(), d->popup->height() ) ) )
        popupPos = mapToGlobal( QPoint( width() - d->popup->width(), -d->popup->height() ) );

    d->popup->popup( popupPos );
}

void KoColorComboBox::colorHasChanged( const QColor &color )
{
    QColor currentColor = color;
    int opacity = d->opacitySlider->value();
    currentColor.setAlpha( opacity );

    KoColor minColor( color, KoColorSpaceRegistry::instance()->rgb8() );
    minColor.setOpacity( 0 );
    KoColor maxColor = minColor;
    maxColor.setOpacity( 255 );

    d->opacitySlider->setColors( minColor, maxColor );

    emit colorChanged( currentColor );

    update();
}

void KoColorComboBox::opacityHasChanged( int opacity )
{
    QColor currentColor = d->colorChooser->color();
    currentColor.setAlpha( opacity );

    emit colorChanged( currentColor );

    update();
}

#include "KoColorComboBox.moc"

