/*  This file is part of the KDE libraries
    Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>
    Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
    Copyright (C) 2006 Casper Boemann <cbr@boemann.dk>

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
#include <math.h>

#include "KoZoomAction.h"

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

#include <klocale.h>
#include <kicon.h>
#include <knuminput.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kdebug.h>

#include <KoZoomMode.h>

KoZoomAction::KoZoomAction( KoZoomMode::Modes zoomModes, const QString& text, bool doSpecialAspectMode, QObject *parent)
    : KSelectAction(text, parent),
    m_zoomModes( zoomModes )
{
    m_slider = 0;
    m_number = 0;
    m_zoomButtonGroup = 0;
    m_doSpecialAspectMode = doSpecialAspectMode;

/*
    m_actualPixels  = new KAction(i18n("Actual Pixels"), this);
    actionCollection()->addAction("actual_pixels", m_actualPixels );
    m_actualPixels->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_0));
    connect(m_actualPixels, SIGNAL(triggered()), this, SLOT(slotActualPixels()));

    m_actualSize = actionCollection()->addAction(KStandardAction::ActualSize,  "actual_size", this, SLOT(slotActualSize()));

    m_fitToCanvas = actionCollection()->addAction(KStandardAction::FitToPage,  "fit_to_canvas", this, SLOT(slotFitToCanvas()));
*/

    setEditable( true );
    setMaxComboViewCount( 15 );

    for(int i = 0; i<33; i++)
        m_sliderLookup[i] = pow(1.1892071, i - 16);

    regenerateItems(0);

    setCurrentAction( i18n( "%1%",  100 ) );
    m_effectiveZoom = 1.0;

    connect( this, SIGNAL( triggered( const QString& ) ), SLOT( triggered( const QString& ) ) );
}

void KoZoomAction::setZoom( const QString& text )
{
    regenerateItems( text );
    setCurrentAction( text );
}

void KoZoomAction::setZoom( double zoom )
{
    setZoom( i18n( "%1%", zoom ) );
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
    m_zoomModes = zoomModes;
    regenerateItems( currentText() );
}

void KoZoomAction::regenerateItems(const QString& zoomString)
{
    bool ok = false;
    int zoom = 0;
    QRegExp regexp( ".*(\\d+).*" ); // "Captured" non-empty sequence of digits
    int pos = regexp.indexIn( zoomString );

    if( pos > -1 )
    {
        zoom = regexp.cap( 1 ).toInt( &ok );
    }

    // where we'll store sorted new zoom values
    QList<int> zoomLevels;
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

    if( ok && zoom > 10 && !zoomLevels.contains( zoom ) )
        zoomLevels << zoom;

    qSort(zoomLevels.begin(), zoomLevels.end());

    // update items with new sorted zoom values
    QStringList values;
    if(m_zoomModes & KoZoomMode::ZOOM_WIDTH)
    {
        values << KoZoomMode::toString(KoZoomMode::ZOOM_WIDTH);
    }
    if(m_zoomModes & KoZoomMode::ZOOM_PAGE)
    {
        values << KoZoomMode::toString(KoZoomMode::ZOOM_PAGE);
    }

    foreach(int value, zoomLevels) {
        values << i18n("%1%", value);
    }

    setItems( values );
}

void KoZoomAction::sliderValueChanged(int value)
{
    setZoom(m_sliderLookup[value]);

    emit zoomChanged( KoZoomMode::ZOOM_CONSTANT, m_sliderLookup[value] );
}

void KoZoomAction::numberValueChanged()
{
    kDebug(30004) << "number widget has changed to " << m_number->text() << endl;

    setZoom(m_number->text());
    double zoom = m_number->text().toDouble()/100.0;

    emit zoomChanged( KoZoomMode::ZOOM_CONSTANT, zoom);
}

void KoZoomAction::zoomIn()
{
    int i=0;
    while(i <= 32 && m_sliderLookup[i] < m_effectiveZoom)
        i++;

    if(i < 32 && m_sliderLookup[i] == m_effectiveZoom)
        i++;
    // else i is the next zoom level already

    double zoom = m_sliderLookup[i];
    setZoom(zoom);
    emit zoomChanged( KoZoomMode::ZOOM_CONSTANT, zoom);
}

void KoZoomAction::zoomOut()
{
    int i=0;
    while(i <= 32 && m_sliderLookup[i] < m_effectiveZoom)
        i++;

    if(i>0)
        i--;

    double zoom = m_sliderLookup[i];
    setZoom(zoom);
    emit zoomChanged( KoZoomMode::ZOOM_CONSTANT, zoom);
}

QWidget * KoZoomAction::createWidget( QWidget * _parent )
{
    // create the custom widget only if we add the action to the status bar
    if( ! qobject_cast<QStatusBar*>(_parent) )
        return KSelectAction::createWidget(_parent);

    QWidget * group = new QWidget(_parent);
    group->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setMinimum(0);
    m_slider->setMaximum(32);
    m_slider->setValue(16);
    m_slider->setSingleStep(1);
    m_slider->setPageStep(1);
    m_slider->setMinimumWidth(80);
    m_slider->setMaximumWidth(80);

    QValidator *validator = new QDoubleValidator(1.0, 1600.0, 0, this);
    m_number = new ExtLineEdit("100", group);
    m_number->setValidator(validator);
    m_number->setMaxLength(5);
    m_number->setMaximumWidth(40);
    m_number->setMaximumHeight(22);
    m_number->setAlignment(Qt::AlignRight);
    m_number->hide();

    QLabel *pctLabel = new QLabel("% ");
    QLabel *numLabel = new QLabel("100");
    numLabel->setMaximumWidth(40);
    numLabel->setMinimumWidth(40);
    numLabel->setIndent(5);
    numLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    numLabel->setContextMenuPolicy(Qt::CustomContextMenu);

    QGridLayout *layout = new QGridLayout();
    int radios=0;
    m_zoomButtonGroup = new QButtonGroup(group);
    m_zoomButtonGroup->setExclusive(true);

    if(m_zoomModes & KoZoomMode::ZOOM_WIDTH)
    {
        QToolButton * fitWidthButton = new QToolButton(group);
        m_zoomButtonGroup->addButton(fitWidthButton, KoZoomMode::ZOOM_WIDTH);
        layout->addWidget(fitWidthButton, 0, radios);
        fitWidthButton->setIcon(KIcon("zoom-width").pixmap(22));
        fitWidthButton->setCheckable(true);
        fitWidthButton->setAutoRaise(true);
        fitWidthButton->setToolTip(i18n("Fit to width"));
        radios++;
    }
    if(m_zoomModes & KoZoomMode::ZOOM_PAGE)
    {
        QToolButton * fitPageButton = new QToolButton(group);
        m_zoomButtonGroup->addButton(fitPageButton, KoZoomMode::ZOOM_PAGE);
        layout->addWidget(fitPageButton, 0, radios);
        fitPageButton->setIcon(KIcon("zoom-page").pixmap(22));
        fitPageButton->setCheckable(true);
        fitPageButton->setAutoRaise(true);
        fitPageButton->setToolTip(i18n("Fit to page/canvas"));
        radios++;
    }


    QToolButton * aspectButton = 0;
    if(m_doSpecialAspectMode)
    {
        aspectButton = new QToolButton(group);
        aspectButton->setIcon(KIcon("zoom-pixels").pixmap(22));
        aspectButton->setCheckable(true);
        aspectButton->setAutoRaise(true);
        aspectButton->setToolTip(i18n("Use same aspect as pixels"));
        connect(aspectButton, SIGNAL(toggled(bool)), this, SIGNAL(aspectModeChanged(bool)));
    }

    layout->addWidget(m_number, 0, radios);
    layout->addWidget(numLabel, 0, radios);
    layout->addWidget(pctLabel, 0, radios+1);
    layout->addWidget(m_slider, 0, radios+2);
    if(m_doSpecialAspectMode)
        layout->addWidget(aspectButton, 0, radios+3);
    layout->setMargin(0);
    layout->setSpacing(0);

    group->setLayout(layout);

    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));
    connect(m_number, SIGNAL(returnPressed()), this, SLOT(numberValueChanged()));
    connect(m_number, SIGNAL(returnPressed()), numLabel, SLOT(show()));
    connect(m_number, SIGNAL(returnPressed()), m_number, SLOT(hide()));
    connect(m_number, SIGNAL(lostFocus()), numLabel, SLOT(show()));
    connect(m_number, SIGNAL(lostFocus()), m_number, SLOT(hide()));
    connect(m_number, SIGNAL(textChanged(const QString & )), numLabel, SLOT(setText(const QString & )));
    connect(numLabel, SIGNAL(customContextMenuRequested(const QPoint &)), m_number, SLOT(show()));
    connect(numLabel, SIGNAL(customContextMenuRequested(const QPoint &)), m_number, SLOT(selectAll()));
    connect(numLabel, SIGNAL(customContextMenuRequested(const QPoint &)), m_number, SLOT(setFocus()));
    connect(numLabel, SIGNAL(customContextMenuRequested(const QPoint &)), numLabel, SLOT(hide()));
    connect(m_zoomButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(zoomModeButtonClicked(int)));
    connect(this, SIGNAL(zoomChanged(KoZoomMode::Mode, double)), this, SLOT(updateWidgets(KoZoomMode::Mode, double)));

    return group;
}

void KoZoomAction::zoomModeButtonClicked(int id)
{
    triggered(KoZoomMode::toString(static_cast<KoZoomMode::Mode>(id)));
}

void KoZoomAction::updateWidgets(KoZoomMode::Mode mode, double zoom)
{
    if(mode != KoZoomMode::ZOOM_CONSTANT) {
        m_zoomButtonGroup->button(mode)->setChecked(true);
    } else {
        QAbstractButton* button = m_zoomButtonGroup->checkedButton();

        if(button) {
            // Let's work around not being able to uncheck when exclusive
            m_zoomButtonGroup->setExclusive(false);
            button->setChecked(false);
            m_zoomButtonGroup->setExclusive(true);
        }

        setEffectiveZoom(zoom);
    }
}

void KoZoomAction::setEffectiveZoom(double zoom)
{
    m_effectiveZoom = zoom;

    if(m_number) {
        if(zoom>0.1)
            m_number->setText(KGlobal::locale()->formatNumber(zoom*100, 0));
        else
            m_number->setText(KGlobal::locale()->formatNumber(zoom*100, 1));
    }

    if(m_slider) {
        int i = 0;
        while(i <= 32 && m_sliderLookup[i] < zoom)
            i++;

        m_slider->blockSignals(true);
        m_slider->setValue(i); // causes sliderValueChanged to be called which does the rest
        m_slider->blockSignals(false);
    }
}


KoZoomAction::ExtLineEdit::ExtLineEdit ( const QString & contents, QWidget * parent) :
   QLineEdit(contents, parent)
{
}

void KoZoomAction::ExtLineEdit::focusOutEvent ( QFocusEvent * event )
{
    QLineEdit::focusOutEvent(event);
    emit lostFocus();
}

#include "KoZoomAction.moc"
