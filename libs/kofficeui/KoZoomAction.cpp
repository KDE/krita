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

#include <KoZoomAction.h>

#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QList>
#include <QToolBar>
#include <QSlider>
#include <QLineEdit>
#include <QRadioButton>
#include <QLabel>
#include <QGridLayout>
#include <QMenu>
#include <QStatusBar>

#include <klocale.h>
#include <kicon.h>
#include <knuminput.h>
#include <kstdaction.h>
#include <kdebug.h>

#include <KoZoomMode.h>

KoZoomAction::KoZoomAction( KoZoomMode::Modes zoomModes, const QString& text, const QIcon& pix,
  const KShortcut& cut, KActionCollection* parent, const char* name ):
  KSelectAction( KIcon(pix), text, parent, name ), m_zoomModes( zoomModes )
{
    setShortcut(cut);

    init(parent);
}

KoZoomAction::KoZoomAction( KoZoomMode::Modes zoomModes, const QString& text, const QString& pix,
  const KShortcut& cut, KActionCollection* parent, const char* name ):
  KSelectAction( KIcon(pix), text, parent, name ), m_zoomModes( zoomModes )
{
    setShortcut(cut);

    init(parent);
}

void KoZoomAction::setZoom( const QString& text )
{
    regenerateItems( text );
    setCurrentAction( text );
}

void KoZoomAction::setZoom( int zoom )
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

    emit zoomChanged( mode, zoom );
}

void KoZoomAction::init(KActionCollection* parent)
{
    QAction *m_zoomIn = KStdAction::zoomIn(this, SLOT(zoomIn()), parent, "zoom_in");
    QAction *m_zoomOut = KStdAction::zoomOut(this, SLOT(zoomOut()), parent, "zoom_out");

/*
    m_actualPixels = new KAction(i18n("Actual Pixels"), actionCollection, "actual_pixels");
    m_actualPixels->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_0));
    connect(m_actualPixels, SIGNAL(triggered()), this, SLOT(slotActualPixels()));

    m_actualSize = KStdAction::actualSize(this, SLOT(slotActualSize()), actionCollection, "actual_size");

    m_fitToCanvas = KStdAction::fitToPage(this, SLOT(slotFitToCanvas()), actionCollection, "fit_to_canvas");
*/

    setEditable( true );
    setMaxComboViewCount( 15 );

    for(int i = 0; i<33; i++)
        m_sliderLookup[i] = int(0.5 + 100 * pow(1.1892071, i - 16));

    regenerateItems(0);

    setCurrentAction( i18n( "%1%",  100 ) );

    connect( this, SIGNAL( triggered( const QString& ) ), SLOT( triggered( const QString& ) ) );
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
    if(m_zoomModes & KoZoomMode::ZOOM_PIXELS)
    {
        values << KoZoomMode::toString(KoZoomMode::ZOOM_PIXELS);
    }

    foreach(int value, zoomLevels) {
        values << i18n("%1%", value);
    }

    setItems( values );
}

void KoZoomAction::sliderValueChanged(int value)
{
    m_number->setText(QString().setNum(m_sliderLookup[value]));

    setZoom(m_sliderLookup[value]);

    emit zoomChanged( KoZoomMode::ZOOM_CONSTANT, m_sliderLookup[value] );
}

void KoZoomAction::numberValueChanged()
{
    kDebug() << "number widget has changed to " << m_number->text() << endl;

    setZoom(m_number->text());
    int zoom = m_number->text().toInt();

    int i=0;
    while(i <= 32 && m_sliderLookup[i] < zoom)
        i++;

    m_slider->blockSignals(true);
    m_slider->setValue(i); // causes sliderValueChanged to be called which does the rest
    m_slider->blockSignals(false);

    emit zoomChanged( KoZoomMode::ZOOM_CONSTANT, zoom);
}

void KoZoomAction::zoomIn()
{
    int zoom = m_number->text().toInt();
kDebug() << m_slider->value() <<endl;

    int i=0;
    while(i <= 32 && m_sliderLookup[i] < zoom)
        i++;
kDebug() << i << " and " << m_sliderLookup[i] << " and " << zoom <<endl;
    if(m_sliderLookup[i] == zoom && i<32)
        i++;
    // else i is the next zoom level already
kDebug() << i << " and " << m_sliderLookup[i] << " and " << zoom <<endl;

   m_slider->setValue(i); // causes sliderValueChanged to be called which does the rest
}

void KoZoomAction::zoomOut()
{
    int zoom = m_number->text().toInt();

    int i=0;
    while(i <= 32 && m_sliderLookup[i] < zoom)
        i++;

    if(i>0)
        i--;

   m_slider->setValue(i); // causes sliderValueChanged to be called which does the rest
}

QWidget * KoZoomAction::createWidget( QWidget * _parent )
{
    QToolBar *parent = qobject_cast<QToolBar *>(_parent);
    if (!parent || !qobject_cast<QStatusBar*>(parent->parent()))
        return KSelectAction::createWidget(_parent);

    QWidget * group = new QWidget(_parent);

    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setFocusPolicy(Qt::NoFocus);
    m_slider->setMinimum(0);
    m_slider->setMaximum(32);
    m_slider->setValue(16);
    m_slider->setSingleStep(1);
    m_slider->setPageStep(1);
    m_slider->setMinimumWidth(80);
    m_slider->setMaximumWidth(80);

    QValidator *validator = new QIntValidator(1, 1600, this);
    m_number = new QLineEdit("100", group);
    m_number->setValidator(validator);
    m_number->setMaxLength(5);
    m_number->setMaximumWidth(40);
    m_number->setAlignment(Qt::AlignRight);

    QLabel *pctLabel = new QLabel("% ");

    QGridLayout *layout = new QGridLayout();
    int radios=0;
    if(m_zoomModes & KoZoomMode::ZOOM_PIXELS)
    {
        m_actualButton= new QRadioButton("AP",group);
        layout->addWidget(m_actualButton, 0, radios);
        radios++;
    }
    if(m_zoomModes & KoZoomMode::ZOOM_WIDTH)
    {
        m_fitWidthButton = new QRadioButton("FW",group);
        layout->addWidget(m_fitWidthButton, 0, radios);
        radios++;
    }
    if(m_zoomModes & KoZoomMode::ZOOM_PAGE)
    {
        m_fitPageButton = new QRadioButton("FP",group);
        layout->addWidget(m_fitPageButton, 0, radios);
        radios++;
    }

    layout->addWidget(m_number, 0, radios);
    layout->addWidget(pctLabel, 0, radios+1);
    layout->addWidget(m_slider, 0, radios+2);
    layout->setMargin(0);
    layout->setSpacing(0);

    group->setLayout(layout);

    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));
    connect(m_number, SIGNAL(returnPressed()), this, SLOT(numberValueChanged()));
    return group;

}

#include "KoZoomAction.moc"
