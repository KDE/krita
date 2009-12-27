/*  This file is part of the KDE libraries
    Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>
    Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
    Copyright (C) 2006-2007 Casper Boemann <cbr@boemann.dk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "KoZoomAction.h"
#include "KoZoomMode.h"
#include "KoZoomInput.h"

#include <QString>
#include <QLocale>
#include <QStringList>
#include <QRegExp>
#include <QList>
#include <QToolBar>
#include <QSlider>
#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include <QGridLayout>
#include <QMenu>
#include <QStatusBar>
#include <QButtonGroup>
#include <QComboBox>

#include <klocale.h>
#include <kicon.h>
#include <knuminput.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kdebug.h>

#include <math.h>

class KoZoomAction::Private
{
public:
    KoZoomMode::Modes zoomModes;
    QSlider *slider;
    qreal sliderLookup[33];
    KoZoomInput* input;
    QToolButton* aspectButton;

    qreal effectiveZoom;

    KoZoomAction::SpecialButtons specialButtons;
};

KoZoomAction::KoZoomAction( KoZoomMode::Modes zoomModes, const QString& text, QObject *parent)
    : KSelectAction(text, parent)
    ,d(new Private)
{
    d->zoomModes = zoomModes;
    d->slider = 0;
    d->input = 0;
    d->specialButtons = 0;
    setIcon(KIcon("zoom-original"));
    setEditable( true );
    setMaxComboViewCount( 15 );

    for(int i = 0; i<33; i++)
        d->sliderLookup[i] = pow(1.1892071, i - 16);

    d->effectiveZoom = 1.0;
    regenerateItems(d->effectiveZoom, true);

    connect( this, SIGNAL( triggered( const QString& ) ), SLOT( triggered( const QString& ) ) );
}

KoZoomAction::~KoZoomAction()
{
    delete d;
}

qreal KoZoomAction::effectiveZoom() const
{
    return d->effectiveZoom;
}

void KoZoomAction::setZoom( qreal zoom )
{
    setEffectiveZoom(zoom);
    regenerateItems( zoom, true );
}

void KoZoomAction::triggered( const QString& text )
{
    QString zoomString = text;
    zoomString = zoomString.remove( '&' );

    KoZoomMode::Mode mode = KoZoomMode::toMode( zoomString );
    int zoom = 0;

    if( mode == KoZoomMode::ZOOM_CONSTANT ) {
        bool ok;
        QRegExp regexp( ".*(\\d+).*" ); // "Captured" non-empty sequence of digits
        int pos = regexp.indexIn( zoomString );

        if( pos > -1 ) {
            zoom = regexp.cap( 1 ).toInt( &ok );

            if( !ok ) {
                zoom = 0;
            }
        }
    }

    emit zoomChanged( mode, zoom/100.0 );
}

void KoZoomAction::setZoomModes( KoZoomMode::Modes zoomModes )
{
    d->zoomModes = zoomModes;
    regenerateItems( d->effectiveZoom );
}

void KoZoomAction::regenerateItems(const qreal zoom, bool asCurrent)
{
    // where we'll store sorted new zoom values
    QList<qreal> zoomLevels;
    zoomLevels << 33;
    zoomLevels << 50;
    zoomLevels << 75;
    zoomLevels << 100;
    zoomLevels << 125;
    zoomLevels << 150;
    zoomLevels << 200;
    zoomLevels << 250;
    zoomLevels << 350;
    zoomLevels << 400;
    zoomLevels << 450;
    zoomLevels << 500;

    if( !zoomLevels.contains( zoom*100 ) )
        zoomLevels << zoom*100;

    qSort(zoomLevels.begin(), zoomLevels.end());

    // update items with new sorted zoom values
    QStringList values;
    if(d->zoomModes & KoZoomMode::ZOOM_WIDTH)
    {
        values << KoZoomMode::toString(KoZoomMode::ZOOM_WIDTH);
    }
    if(d->zoomModes & KoZoomMode::ZOOM_PAGE)
    {
        values << KoZoomMode::toString(KoZoomMode::ZOOM_PAGE);
    }

    foreach(qreal value, zoomLevels) {
        if(value>10.0)
            values << i18n("%1%", KGlobal::locale()->formatNumber(value, 0));
        else
            values << i18n("%1%", KGlobal::locale()->formatNumber(value, 1));
    }

    setItems( values );

    if(d->input)
        d->input->setZoomLevels(values);

    if(asCurrent)
    {
        QString valueString;
        if(zoom*100>10.0)
            valueString = i18n("%1%", KGlobal::locale()->formatNumber(zoom*100, 0));
        else
            valueString = i18n("%1%", KGlobal::locale()->formatNumber(zoom*100, 1));

        setCurrentAction(valueString);

        if(d->input)
            d->input->setCurrentZoomLevel(valueString);
    }
}

void KoZoomAction::sliderValueChanged(int value)
{
    setZoom(d->sliderLookup[value]);

    emit zoomChanged( KoZoomMode::ZOOM_CONSTANT, d->sliderLookup[value] );
}

void KoZoomAction::zoomIn()
{
    int i=0;
    while(i <= 32 && d->sliderLookup[i] < d->effectiveZoom)
        i++;

    if(i < 32 && d->sliderLookup[i] == d->effectiveZoom)
        i++;
    // else i is the next zoom level already

    qreal zoom = d->sliderLookup[i];
    setZoom(zoom);
    emit zoomChanged( KoZoomMode::ZOOM_CONSTANT, zoom);
}

void KoZoomAction::zoomOut()
{
    int i=0;
    while(i <= 32 && d->sliderLookup[i] < d->effectiveZoom)
        i++;

    if(i>0)
        i--;

    qreal zoom = d->sliderLookup[i];
    setZoom(zoom);
    emit zoomChanged( KoZoomMode::ZOOM_CONSTANT, zoom);
}

QWidget * KoZoomAction::createWidget( QWidget * _parent )
{
    // create the custom widget only if we add the action to the status bar
    if( ! qobject_cast<QStatusBar*>(_parent) )
        return KSelectAction::createWidget(_parent);

    QWidget * group = new QWidget(_parent);
    QHBoxLayout *layout = new QHBoxLayout(group);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setMargin(0);
    layout->setSpacing(0);

    // this is wrong; createWidget() implies this is a factory method, so we should be able to be called
    // multiple times without problems.  The 'new' here means we can't do that.
    // TODO refactor this method to use connections instead of d-pointer members to communicate so it becomes reentrant.
    d->input = new KoZoomInput(group);
    regenerateItems( d->effectiveZoom, true );
    connect(d->input, SIGNAL(zoomLevelChanged(const QString&)), this, SLOT(triggered(const QString&)));
    layout->addWidget(d->input);

    d->slider = new QSlider(Qt::Horizontal);
    d->slider->setToolTip(i18n("Zoom"));
    d->slider->setMinimum(0);
    d->slider->setMaximum(32);
    d->slider->setValue(16);
    d->slider->setSingleStep(1);
    d->slider->setPageStep(1);
    d->slider->setMinimumWidth(80);
    d->slider->setMaximumWidth(80);
    layout->addWidget(d->slider);

    if (d->specialButtons & AspectMode) {
        d->aspectButton = new QToolButton(group);
        d->aspectButton->setIcon(KIcon("zoom-pixels").pixmap(22));
        d->aspectButton->setCheckable(true);
        d->aspectButton->setChecked(true);
        d->aspectButton->setAutoRaise(true);
        d->aspectButton->setToolTip(i18n("Use same aspect as pixels"));
        connect(d->aspectButton, SIGNAL(toggled(bool)), this, SIGNAL(aspectModeChanged(bool)));
        layout->addWidget(d->aspectButton);
    }
    if (d->specialButtons & ZoomToSelection) {
        QToolButton * zoomToSelectionButton = new QToolButton(group);
        zoomToSelectionButton->setIcon(KIcon("zoom-selection").pixmap(22));
        zoomToSelectionButton->setAutoRaise(true);
        zoomToSelectionButton->setToolTip(i18n("Zoom to Selection"));
        connect(zoomToSelectionButton, SIGNAL(clicked(bool)), this, SIGNAL(zoomedToSelection()));
        layout->addWidget(zoomToSelectionButton);
    }
    if (d->specialButtons & ZoomToAll) {
        QToolButton * zoomToAllButton = new QToolButton(group);
        zoomToAllButton->setIcon(KIcon("zoom-all").pixmap(22));
        zoomToAllButton->setAutoRaise(true);
        zoomToAllButton->setToolTip(i18n("Zoom to All"));
        connect(zoomToAllButton, SIGNAL(clicked(bool)), this, SIGNAL(zoomedToAll()));
        layout->addWidget(zoomToAllButton);
    }

    connect(d->slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));

    return group;
}

void KoZoomAction::setEffectiveZoom(qreal zoom)
{
    if(d->effectiveZoom == zoom)
        return;
    d->effectiveZoom = zoom;

    if(d->slider) {
        int i = 0;
        while(i <= 32 && d->sliderLookup[i] < zoom)
            i++;

        d->slider->blockSignals(true);
        d->slider->setValue(i); // causes sliderValueChanged to be called which does the rest
        d->slider->blockSignals(false);
    }
}

void KoZoomAction::setSelectedZoomMode( KoZoomMode::Mode mode )
{
    QString modeString(KoZoomMode::toString(mode));
    setCurrentAction(modeString);

    if (d->input)
        d->input->setCurrentZoomLevel(modeString);
}

void KoZoomAction::setSpecialButtons( SpecialButtons buttons )
{
    d->specialButtons = buttons;
}

void KoZoomAction::setAspectMode(bool status)
{
    /**
     * In the first case, the signal will be emitted
     * by the button itself, in the second we help it a bit
     *
     * It means that the result of this function is
     * ALWAYS an emitted signal
     */
    if(d->aspectButton->isChecked() != status)
        d->aspectButton->setChecked(status);
    else
        emit aspectModeChanged(status);
}

#include <KoZoomAction.moc>
