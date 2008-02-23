/*
 *  Copyright (c) 2004,2007 Cyrille Berger <cberger@cberger.net>
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
 
#include "kis_auto_brush_widget.h"
#include <KoImageResource.h>
#include <kis_debug.h>
#include <QSpinBox>
#include <QToolButton>
#include <QImage>
#include <QComboBox>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>


KisAutoBrushWidget::KisAutoBrushWidget(QWidget *parent, const char* name, const QString& caption) : KisWdgAutobrush(parent, name)
{
    setWindowTitle(caption);

    m_linkSize = true;
    m_linkFade = true;
    
    linkFadeToggled(m_linkSize);
    linkSizeToggled(m_linkFade);

    connect(bnLinkSize, SIGNAL(toggled(bool)), this, SLOT(linkSizeToggled( bool )));
    connect(bnLinkFade, SIGNAL(toggled(bool)), this, SLOT(linkFadeToggled( bool )));
    
    connect((QObject*)comboBoxShape, SIGNAL(activated(int)), this, SLOT(paramChanged()));
    spinBoxWidth->setMinimum(1);
    connect(spinBoxWidth,SIGNAL(valueChanged(int)),this,SLOT(spinBoxWidthChanged(int)));
    spinBoxHeigth->setMinimum(1);
    connect(spinBoxHeigth,SIGNAL(valueChanged(int)),this,SLOT(spinBoxHeigthChanged(int)));
    spinBoxHorizontal->setMinimum(0);
    connect(spinBoxHorizontal,SIGNAL(valueChanged(int)),this,SLOT(spinBoxHorizontalChanged(int)));
    spinBoxVertical->setMinimum(0);
    connect(spinBoxVertical,SIGNAL(valueChanged(int)),this,SLOT(spinBoxVerticalChanged(int)));

    m_brush = QImage(1, 1, QImage::Format_RGB32);
    paramChanged();


    connect(brushPreview, SIGNAL(clicked()), SLOT(paramChanged()));

}

void KisAutoBrushWidget::resizeEvent ( QResizeEvent * )
{
    brushPreview->setMinimumHeight(brushPreview->width()); // dirty hack !
    brushPreview->setMaximumHeight(brushPreview->width()); // dirty hack !
}

void KisAutoBrushWidget::activate()
{
    paramChanged();
}

void KisAutoBrushWidget::paramChanged()
{
    qint32 fh = qMin( spinBoxWidth->value()/2, spinBoxHorizontal->value() ) ;
    qint32 fv = qMin( spinBoxHeigth->value()/2, spinBoxVertical->value() );
    KisAutobrushShape* kas;

    if(comboBoxShape->currentIndex() == 0) // use index compare instead of comparing a translatable string
    {
        kas = new KisAutobrushCircleShape(spinBoxWidth->value(),  spinBoxHeigth->value(), fh, fv);
        Q_CHECK_PTR(kas);

    } else {
        kas = new KisAutobrushRectShape(spinBoxWidth->value(),  spinBoxHeigth->value(), fh, fv);
        Q_CHECK_PTR(kas);

    }
    m_brush = kas->createBrush();

    QPixmap p;
    QImage pi(m_brush);
    double coeff = 1.0;
    int bPw = brushPreview->width()-3;
    if(pi.width() > bPw)
    {
        coeff =  bPw /(double)pi.width();
    }
    int bPh = brushPreview->height()-3;
    if(pi.height() > coeff * bPh)
    {
        coeff = bPh /(double)pi.height();
    }
    if( coeff < 1.0)
    {
        pi = pi.scaled( (int)(coeff * pi.width()) , (int)(coeff * pi.height()),  Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    
    p = QPixmap::fromImage(pi);
    brushPreview->setIcon(QIcon(p));
    KisAutoBrush * resource = new KisAutoBrush(kas);
    Q_CHECK_PTR(resource);

    emit(activatedResource(resource));
}
void KisAutoBrushWidget::spinBoxWidthChanged(int a)
{
    spinBoxHorizontal->setMaximum(a/2);
    if(m_linkSize)
    {
        spinBoxHeigth->setValue(a);
        spinBoxVertical->setMaximum(a/2);
    }
    this->paramChanged();
}
void KisAutoBrushWidget::spinBoxHeigthChanged(int a)
{
    spinBoxVertical->setMaximum(a/2);
    if(m_linkSize)
    {
        spinBoxWidth->setValue(a);
        spinBoxHorizontal->setMaximum(a/2);
    }
    this->paramChanged();
}
void KisAutoBrushWidget::spinBoxHorizontalChanged(int a)
{
    if(m_linkFade)
        spinBoxVertical->setValue(a);
    this->paramChanged();
}
void KisAutoBrushWidget::spinBoxVerticalChanged(int a)
{
    if(m_linkFade)
        spinBoxHorizontal->setValue(a);
    this->paramChanged();
}

void KisAutoBrushWidget::linkSizeToggled(bool b)
{
    m_linkSize = b;

    KoImageResource kir;
    if (b) {
        bnLinkSize->setIcon(QIcon(kir.chain()));
    }
    else {
        bnLinkSize->setIcon(QIcon(kir.chainBroken()));
    }
}

void KisAutoBrushWidget::linkFadeToggled(bool b)
{
    m_linkFade = b;

    KoImageResource kir;
    if (b) {
        bnLinkFade->setIcon(QIcon(kir.chain()));
    }
    else {
        bnLinkFade->setIcon(QIcon(kir.chainBroken()));
    }
}

#include "kis_auto_brush_widget.moc"
