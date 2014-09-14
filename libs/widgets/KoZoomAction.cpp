/*  This file is part of the KDE libraries
    Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>
    Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
    Copyright (C) 2006-2007 C. Boemann <cbo@boemann.dk>

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
#include "KoZoomWidget.h"

#include <KoIcon.h>

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
#include <knuminput.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kdebug.h>

#include <math.h>

class KoZoomAction::Private
{
public:

    Private() {}

    KoZoomMode::Modes zoomModes;
    QList<qreal> sliderLookup;

    qreal effectiveZoom;

    KoZoomAction::SpecialButtons specialButtons;

    QList<qreal> generateSliderZoomLevels() const;
    QList<qreal> filterMenuZoomLevels(const QList<qreal> &zoomLevels) const;
};

QList<qreal> KoZoomAction::Private::generateSliderZoomLevels() const
{
    QList<qreal> zoomLevels;

    qreal defaultZoomStep = sqrt(2.0);

    zoomLevels << 0.25 / 2.0;
    zoomLevels << 0.25 / 1.5;
    zoomLevels << 0.25;
    zoomLevels << 1.0 / 3.0;
    zoomLevels << 0.5;
    zoomLevels << 2.0 / 3.0;
    zoomLevels << 1.0;

    for (qreal zoom = zoomLevels.first() / defaultZoomStep;
         zoom > KoZoomMode::minimumZoom();
         zoom /= defaultZoomStep) {

        zoomLevels.prepend(zoom);
    }

    for (qreal zoom = zoomLevels.last() * defaultZoomStep;
         zoom < KoZoomMode::maximumZoom();
         zoom *= defaultZoomStep) {

        zoomLevels.append(zoom);
    }

    return zoomLevels;
}

QList<qreal> KoZoomAction::Private::filterMenuZoomLevels(const QList<qreal> &zoomLevels) const
{
    QList<qreal> filteredZoomLevels;

    foreach(qreal zoom, zoomLevels) {
        if (zoom >= 0.2 && zoom <= 10) {
            filteredZoomLevels << zoom;
        }
    }

    return filteredZoomLevels;
}

KoZoomAction::KoZoomAction( KoZoomMode::Modes zoomModes, const QString& text, QObject *parent)
    : KSelectAction(text, parent)
    , d(new Private)
{
    d->zoomModes = zoomModes;
    d->specialButtons = 0;
    setIcon(koIcon("zoom-original"));
    setEditable( true );
    setMaxComboViewCount( 15 );

    d->sliderLookup = d->generateSliderZoomLevels();

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
    regenerateItems( d->effectiveZoom, true );
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
    QList<qreal> zoomLevels = d->filterMenuZoomLevels(d->sliderLookup);

    if( !zoomLevels.contains( zoom ) )
        zoomLevels << zoom;

    qSort(zoomLevels.begin(), zoomLevels.end());

    // update items with new sorted zoom values
    QStringList values;
    if(d->zoomModes & KoZoomMode::ZOOM_WIDTH) {
        values << KoZoomMode::toString(KoZoomMode::ZOOM_WIDTH);
    }
    if(d->zoomModes & KoZoomMode::ZOOM_TEXT) {
        values << KoZoomMode::toString(KoZoomMode::ZOOM_TEXT);
    }
    if(d->zoomModes & KoZoomMode::ZOOM_PAGE) {
        values << KoZoomMode::toString(KoZoomMode::ZOOM_PAGE);
    }

    foreach(qreal value, zoomLevels) {
        if(value>10.0)
            values << i18n("%1%", KGlobal::locale()->formatNumber(value * 100, 0));
        else
            values << i18n("%1%", KGlobal::locale()->formatNumber(value * 100, 1));
    }

    setItems( values );

    emit zoomLevelsChanged(values);

    if(asCurrent)
    {
        QString valueString;
        if(zoom*100>10.0)
            valueString = i18n("%1%", KGlobal::locale()->formatNumber(zoom*100, 0));
        else
            valueString = i18n("%1%", KGlobal::locale()->formatNumber(zoom*100, 1));

        setCurrentAction(valueString);

        emit currentZoomLevelChanged(valueString);
    }
}

void KoZoomAction::sliderValueChanged(int value)
{
    setZoom(d->sliderLookup[value]);

    emit zoomChanged( KoZoomMode::ZOOM_CONSTANT, d->sliderLookup[value] );
}

qreal KoZoomAction::nextZoomLevel() const
{
    const qreal eps = 1e-5;
    int i = 0;
    while (d->effectiveZoom > d->sliderLookup[i] - eps &&
           i < d->sliderLookup.size() - 1) i++;

    return qMax(d->effectiveZoom, d->sliderLookup[i]);
}

qreal KoZoomAction::prevZoomLevel() const
{
    const qreal eps = 1e-5;
    int i = d->sliderLookup.size() - 1;
    while (d->effectiveZoom < d->sliderLookup[i] + eps && i > 0) i--;

    return qMin(d->effectiveZoom, d->sliderLookup[i]);
}

void KoZoomAction::zoomIn()
{
    qreal zoom = nextZoomLevel();

    if (zoom > d->effectiveZoom) {
        setZoom(zoom);
        emit zoomChanged( KoZoomMode::ZOOM_CONSTANT, d->effectiveZoom);
    }
}

void KoZoomAction::zoomOut()
{
    qreal zoom = prevZoomLevel();

    if (zoom < d->effectiveZoom) {
        setZoom(zoom);
        emit zoomChanged( KoZoomMode::ZOOM_CONSTANT, d->effectiveZoom);
    }
}

QWidget * KoZoomAction::createWidget(QWidget *parent)
{
    // create the custom widget only if we add the action to the status bar
    if (!qobject_cast<QStatusBar*>(parent))
        return KSelectAction::createWidget(parent);

    KoZoomWidget* zoomWidget = new KoZoomWidget(parent, d->specialButtons, d->sliderLookup.size() - 1);
    connect(this, SIGNAL(zoomLevelsChanged(QStringList)), zoomWidget, SLOT(setZoomLevels(QStringList)));
    connect(this, SIGNAL(currentZoomLevelChanged(QString)), zoomWidget, SLOT(setCurrentZoomLevel(QString)));
    connect(this, SIGNAL(sliderChanged(int)), zoomWidget, SLOT(setSliderValue(int)));
    connect(this, SIGNAL(aspectModeChanged(bool)), zoomWidget, SLOT(setAspectMode(bool)));
    connect(zoomWidget, SIGNAL(sliderValueChanged(int)), this, SLOT(sliderValueChanged(int)));
    connect(zoomWidget, SIGNAL(zoomLevelChanged(const QString&)), this, SLOT(triggered(const QString&)));
    connect(zoomWidget, SIGNAL(aspectModeChanged(bool)), this, SIGNAL(aspectModeChanged(bool)));
    connect(zoomWidget, SIGNAL(zoomedToSelection()), this, SIGNAL(zoomedToSelection()));
    connect(zoomWidget, SIGNAL(zoomedToAll()), this, SIGNAL(zoomedToAll()));
    regenerateItems( d->effectiveZoom, true );
    syncSliderWithZoom();
    return zoomWidget;
}

void KoZoomAction::setEffectiveZoom(qreal zoom)
{
    if(d->effectiveZoom == zoom)
        return;

    zoom = KoZoomMode::clampZoom(zoom);
    d->effectiveZoom = zoom;
    syncSliderWithZoom();
}

void KoZoomAction::setSelectedZoomMode( KoZoomMode::Mode mode )
{
    QString modeString(KoZoomMode::toString(mode));
    setCurrentAction(modeString);

    emit currentZoomLevelChanged(modeString);
}

void KoZoomAction::setSpecialButtons( SpecialButtons buttons )
{
    d->specialButtons = buttons;
}

void KoZoomAction::setAspectMode(bool status)
{
    emit aspectModeChanged(status);
}

void KoZoomAction::syncSliderWithZoom()
{
    const qreal eps = 1e-5;
    int i = d->sliderLookup.size() - 1;
    while (d->effectiveZoom < d->sliderLookup[i] + eps && i > 0) i--;
    
    emit sliderChanged(i);
}

#include <KoZoomAction.moc>
