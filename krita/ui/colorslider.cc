/*
 *  colorslider.cc - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <kdebug.h>

#include <qpainter.h>
#include <qpen.h>
#include <qcursor.h>

#include "colorframe.h"
#include "colorslider.h"


ColorSlider::ColorSlider(QWidget *parent, int _colorSliderType)
    : QWidget(parent)
{
    m_ColorSliderType = _colorSliderType;

    m_pColorFrame = new ColorFrame(this, _colorSliderType);
    m_pSlider = new SliderWidget(this);

    m_min = 0;

    if(_colorSliderType == 1)
        m_max = 359;
    else
        m_max = 255;

    m_value = 0;

    connect(m_pSlider, SIGNAL(positionChanged(int)),
		  this, SLOT(slotSliderMoved(int)));

    connect(m_pColorFrame, SIGNAL(clicked(const QPoint&)),
		  this, SLOT(slotFrameClicked(const QPoint&)));
}


ColorSlider::~ColorSlider()
{
    delete m_pColorFrame;
    delete m_pSlider;
}


int ColorSlider::minValue()
{
    return  m_min;
}


int ColorSlider::maxValue()
{
    return  m_max;
}


void ColorSlider::slotSetRange(int min, int max)
{
    if (min >= max) return;

    m_min = min;
    m_max = max;
}


void ColorSlider::resizeEvent (QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    // m_pSlider->width()/2 * 2 seems stupid but is not because for example
    // m_pSlider->width() == 11 I get 10.
    m_pColorFrame->setGeometry(m_pSlider->width()/2, 0,
        width()- m_pSlider->width()/2 * 2, height() - m_pSlider->height());
    slotSetValue(m_value);
}


void ColorSlider::slotSetColor1(const QColor& c)
{
    if(m_ColorSliderType == 0)
        m_pColorFrame->slotSetColor1(c);
}


void ColorSlider::slotSetColor2(const QColor& c)
{
    if(m_ColorSliderType == 0)
        m_pColorFrame->slotSetColor2(c);
}

void ColorSlider::slotSetHue(int _hue)
{
    if(m_ColorSliderType == 2 || m_ColorSliderType == 3)
        m_pColorFrame->slotSetHue(_hue);
}

void ColorSlider::slotSetSaturation(int /*_sat*/)
{
    //m_pColorFrame->slotSetSaturation(_sat);
}

void ColorSlider::slotSetVal(int /*_val*/)
{
    //m_pColorFrame->slotSetValue(_val);
}


void ColorSlider::slotSetValue(int value)
{
    if (value < m_min) value = m_min;
    if (value > m_max) value = m_max;

    m_value = value;

    int range = m_max - m_min;
    float v = value;
    if (m_min < 0) v += -m_min;

    float factor = v /range;
    int x = static_cast<int>(factor * m_pColorFrame->contentsRect().width());

    m_pSlider->move(QPoint(x , height() - m_pSlider->height()));
}


void ColorSlider::slotSliderMoved(int x)
{
    if (x < 0)
	    x = 0;
    if (x > m_pColorFrame->contentsRect().width())
	    x = m_pColorFrame->contentsRect().width();

    //kdDebug(0) << "x: " << x << endl;
    float factor = x;
    factor /= m_pColorFrame->contentsRect().width();
    //kdDebug(0) << "factor: " << factor << endl;

    int range = m_max - m_min;
    //kdDebug(0) << "range: " << range << endl;

    m_value = static_cast<int>(factor * range);
    //kdDebug(0) << "m_value: " << m_value << endl;

    emit valueChanged(m_value);
    emit colorSelected(m_pColorFrame->colorAt(QPoint(x,
        m_pColorFrame->contentsRect().height()/2)));
}


void ColorSlider::slotFrameClicked(const QPoint& p)
{
    QPoint local = m_pColorFrame->mapToParent(p);
    QPoint pos = QPoint(local.x() - m_pSlider->width()/2,
        height() - m_pSlider->height());

    if (pos.x() < 0)
	    pos.setX(0);
    else if (pos.x() > width() - m_pSlider->width())
	    pos.setX(width() - m_pSlider->width());

    m_pSlider->move(pos);
    slotSliderMoved(pos.x());
}


void ColorSlider::mousePressEvent (QMouseEvent *e)
{
    if (e->button() & LeftButton)
    {
	    QPoint pos = QPoint(e->pos().x() - m_pSlider->width()/2,
            height() - m_pSlider->height());

	    if (pos.x() < 0)
		    pos.setX(0);
	    else if (pos.x() > width() - m_pSlider->width())
		    pos.setX(width() - m_pSlider->width());

	    m_pSlider->move(pos);
	    slotSliderMoved(pos.x());
	}
    else
        QWidget::mousePressEvent(e);
}


SliderWidget::SliderWidget(QWidget *parent) : QWidget(parent)
{
    m_dragging = false;
    setFixedHeight(6);
    setFixedWidth(11);
}


void SliderWidget::paintEvent (QPaintEvent *)
{
    QPainter p;
    QPen pen(black, 1);
    p.begin(this);

    p.setPen(pen);
    p.drawLine(0, 5, 5, 0);
    p.drawLine(10, 5, 5, 0);
    p.drawLine(0, 5, 10, 5);
    p.end();
}


void SliderWidget::mousePressEvent (QMouseEvent *e)
{
    if (e->button() & LeftButton)
    {
        m_myPos = e->pos();
	    m_dragging = true;
	}
    else
        QWidget::mousePressEvent(e);
}


void SliderWidget::mouseReleaseEvent (QMouseEvent *e)
{
    if (e->button() & LeftButton)
	    m_dragging = false;
    else
        QWidget::mouseReleaseEvent(e);
}


void SliderWidget::mouseMoveEvent (QMouseEvent *e)
{
    if (m_dragging)
    {
	    QWidget *p = parentWidget();
	    if (!p) return;

	    QPoint newPos = p->mapFromGlobal(QCursor::pos()) - m_myPos;

	    // don't drag vertically :-)
	    newPos.setY(pos().y());

	    if (newPos.x() < 0)
		    newPos.setX(0);
	    if (newPos.x() > p->width()- width())
		    newPos.setX(p->width()- width());

	    move(newPos);
	    emit positionChanged(newPos.x());
    }
    else
	    QWidget::mouseMoveEvent(e);
}


#include "colorslider.moc"
