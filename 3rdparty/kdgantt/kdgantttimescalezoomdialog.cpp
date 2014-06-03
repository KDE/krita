/* This file is part of the Calligra project
 * Copyright (c) 2008 Dag Andersen <kplato@kde.org>
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

#include "kdgantttimescalezoomdialog.h"

#include "kdganttdatetimegrid.h"

#include <QLocale>
#include <QHBoxLayout>
#include <QToolButton>
#include <qmath.h>

namespace KDGantt {

/*!\class KDGantt::Slider
 * \internal
 */

Slider::Slider( QWidget *parent )
    : QSlider( parent ), m_hide( false ), m_grid( 0 )
{
    setOrientation( Qt::Horizontal );
    setPageStep( 5 );
    setMaximum( 125 );
    connect(this, SIGNAL(valueChanged(int)), SLOT(sliderValueChanged(int)));
}

void Slider::setEnableHideOnLeave( bool hide )
{
    m_hide = hide;
}

void Slider::setGrid( DateTimeGrid *grid )
{
    m_grid = grid;
    if ( grid ) {
        int pos = -1; // daywidth always >= 0.1
        for ( qreal dw = grid->dayWidth(); dw >= 0.1 && pos < maximum(); ++pos ) {
            dw *= 1.0 / 1.1;
        }
        blockSignals( true );
        setValue( pos );
        blockSignals( false );
    }
}

void Slider::leaveEvent( QEvent *e )
{
    if ( m_hide ) {
        hide();
    }
    QSlider::leaveEvent( e );
}

void Slider::sliderValueChanged( int value )
{
    if ( m_grid ) {
        m_grid->setDayWidth( qPow( 1.1, value ) * 0.1 );
    }
}

/*!\class KDGantt::TimeScaleZoomDialog
 * \internal
 */
TimeScaleZoomDialog::TimeScaleZoomDialog( QWidget *parent )
    : QDialog( parent )
{
    pane.setupUi( this );
    zoom = pane.zoom;
}

void Ui_TimeScaleZoomPane::setupUi(QDialog *KDGantt__TimeScaleZoomPane)
{
    if (KDGantt__TimeScaleZoomPane->objectName().isEmpty())
        KDGantt__TimeScaleZoomPane->setObjectName(QString::fromUtf8("KDGantt__TimeScaleZoomPane"));
    KDGantt__TimeScaleZoomPane->resize(152, 55);
    KDGantt__TimeScaleZoomPane->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
    KDGantt__TimeScaleZoomPane->setModal(true);
    horizontalLayout = new QHBoxLayout(KDGantt__TimeScaleZoomPane);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

    zoom = new Slider(KDGantt__TimeScaleZoomPane);
    horizontalLayout->addWidget(zoom);

    retranslateUi(KDGantt__TimeScaleZoomPane);

    QMetaObject::connectSlotsByName(KDGantt__TimeScaleZoomPane);
} // setupUi

void Ui_TimeScaleZoomPane::retranslateUi(QDialog *KDGantt__TimeScaleZoomPane)
{
    KDGantt__TimeScaleZoomPane->setWindowTitle(QDialog::tr("Zoom"));
    Q_UNUSED(KDGantt__TimeScaleZoomPane);
} // retranslateUi

} // namespace KDGantt

#include "moc_kdgantttimescalezoomdialog.cpp"

