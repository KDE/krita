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
#include "KoZoomWidget.h"

#include <KoIcon.h>

#include <QString>
#include <QLocale>
#include <QStringList>
#include <QRegExp>
#include <QList>
#include <QSlider>
#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include <QGridLayout>
#include <QMenu>
#include <QStatusBar>
#include <QButtonGroup>
#include <QComboBox>


#include <klocalizedstring.h>
#include <WidgetsDebug.h>

#include <math.h>

class Q_DECL_HIDDEN KoZoomAction::Private
{
public:

    Private(KoZoomAction *_parent)
        : parent(_parent)
        , minimumZoomValue(-1)
        , maximumZoomValue(-1)
    {}

    KoZoomAction *parent;

    KoZoomMode::Modes zoomModes;
    QList<qreal> sliderLookup;

    qreal effectiveZoom;

    KoZoomAction::SpecialButtons specialButtons;

    QList<qreal> generateSliderZoomLevels() const;
    QList<qreal> filterMenuZoomLevels(const QList<qreal> &zoomLevels) const;

    qreal minimumZoomValue;
    qreal maximumZoomValue;
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
         zoom > parent->minimumZoom();
         zoom /= defaultZoomStep) {

        zoomLevels.prepend(zoom);
    }

    for (qreal zoom = zoomLevels.last() * defaultZoomStep;
         zoom < parent->maximumZoom();
         zoom *= defaultZoomStep) {

        zoomLevels.append(zoom);
    }

    return zoomLevels;
}

QList<qreal> KoZoomAction::Private::filterMenuZoomLevels(const QList<qreal> &zoomLevels) const
{
    QList<qreal> filteredZoomLevels;

    Q_FOREACH (qreal zoom, zoomLevels) {
        if (zoom >= 0.2 && zoom <= 10) {
            filteredZoomLevels << zoom;
        }
    }

    return filteredZoomLevels;
}

KoZoomAction::KoZoomAction(KoZoomMode::Modes zoomModes, const QString& text, QObject *parent)
    : KSelectAction(text, parent)
    , d(new Private(this))
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

void KoZoomAction::setZoom(qreal zoom)
{
    setEffectiveZoom(zoom);
    regenerateItems(d->effectiveZoom, true);
}

void KoZoomAction::triggered(const QString& text)
{
    QString zoomString = text;
    zoomString = zoomString.remove( '&' );

    KoZoomMode::Mode mode = KoZoomMode::toMode(zoomString);
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

    std::sort(zoomLevels.begin(), zoomLevels.end());

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

    Q_FOREACH (qreal value, zoomLevels) {
        const qreal valueInPercent = value * 100;
        const int precision = (value > 10.0) ? 0 : 1;

        values << i18n("%1%", QLocale().toString(valueInPercent, 'f', precision));
    }

    setItems( values );

    emit zoomLevelsChanged(values);

    if(asCurrent)
    {
        const qreal zoomInPercent = zoom * 100;
        // TODO: why zoomInPercent and not zoom here? different from above
        const int precision = (zoomInPercent > 10.0) ? 0 : 1;

        const QString valueString = i18n("%1%", QLocale().toString(zoomInPercent, 'f', precision));

        setCurrentAction(valueString);

        emit currentZoomLevelChanged(valueString);
    }
}

void KoZoomAction::sliderValueChanged(int value)
{
    setZoom(d->sliderLookup[value]);

    emit zoomChanged(KoZoomMode::ZOOM_CONSTANT, d->sliderLookup[value]);
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
        emit zoomChanged(KoZoomMode::ZOOM_CONSTANT, d->effectiveZoom);
    }
}

void KoZoomAction::zoomOut()
{
    qreal zoom = prevZoomLevel();

    if (zoom < d->effectiveZoom) {
        setZoom(zoom);
        emit zoomChanged(KoZoomMode::ZOOM_CONSTANT, d->effectiveZoom);
    }
}

QWidget * KoZoomAction::createWidget(QWidget *parent)
{
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

    zoom = clampZoom(zoom);
    d->effectiveZoom = zoom;
    syncSliderWithZoom();
}

void KoZoomAction::setSelectedZoomMode(KoZoomMode::Mode mode)
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

qreal KoZoomAction::minimumZoom()
{
    if (d->minimumZoomValue < 0) {
        return KoZoomMode::minimumZoom();
    }
    return d->minimumZoomValue;
}

qreal KoZoomAction::maximumZoom()
{
    if (d->maximumZoomValue < 0) {
        return KoZoomMode::maximumZoom();
    }
    return d->maximumZoomValue;
}

qreal KoZoomAction::clampZoom(qreal zoom)
{
    return qMin(maximumZoom(), qMax(minimumZoom(), zoom));
}

void KoZoomAction::setMinimumZoom(qreal zoom)
{
    Q_ASSERT(zoom > 0.0f);
    KoZoomMode::setMinimumZoom(zoom);
    d->minimumZoomValue = zoom;
    d->generateSliderZoomLevels();
    d->sliderLookup = d->generateSliderZoomLevels();
    regenerateItems(d->effectiveZoom, true);
    syncSliderWithZoom();
}

void KoZoomAction::setMaximumZoom(qreal zoom)
{
    Q_ASSERT(zoom > 0.0f);
    KoZoomMode::setMaximumZoom(zoom);
    d->maximumZoomValue = zoom;
    d->sliderLookup = d->generateSliderZoomLevels();
    regenerateItems(d->effectiveZoom, true);
    syncSliderWithZoom();
}
