/* This file is part of the KDE project
  Copyright (c) 1999 Matthias Elter (me@kde.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "koColorSlider.h"

#include <QPainter>
#include <QCursor>
#include <QPen>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3Frame>
#include <QResizeEvent>
#include <QPaintEvent>

#include <kdebug.h>
#include <kpixmapeffect.h>

KoColorFrame::KoColorFrame(QWidget *parent):
Q3Frame(parent)
{
  setFrameStyle(Panel | Sunken);
  setBackgroundMode(Qt::NoBackground);

  /* default values */
  mC1 = QColor(0, 0, 0);
  mC2 = QColor(255, 255, 255);

  mColorChanged = false;
  mPixChanged = false;
  mDragging = false;
}

const QColor KoColorFrame::colorAt(const QPoint &p)
{
  if(mPixChanged)
  {
    mImage = mPixmap.convertToImage();
    mPixChanged = false;
  }

  if(p.x() >= mPixmap.width() || p.y() >= mPixmap.height())
    return QColor(255,255,255);

  return QColor(mImage.pixel(p.x(), p.y()));
}

void KoColorFrame::slotSetColor1(const QColor &c)
{
  mC1 = c;
  mColorChanged = true;
  mPixChanged = true;
  repaint();
}

void KoColorFrame::slotSetColor2(const QColor &c)
{
  mC2 = c;
  mColorChanged = true;
  repaint();
}

void KoColorFrame::drawContents(QPainter *p)
{
  QRect r = contentsRect();

  if((mPixmap.size() != r.size()) || mColorChanged)
  {
    mPixmap.resize(r.width() + 1, r.height() + 1);
    KPixmapEffect::gradient(mPixmap, mC1, mC2, KPixmapEffect::HorizontalGradient);
    mColorChanged = false;
    mPixChanged = true;
  }

  p->drawPixmap(r.left(), r.top(), mPixmap);
}

void KoColorFrame::mousePressEvent(QMouseEvent *e)
{
  if(e->button() & Qt::LeftButton)
  {
    emit clicked(e->pos());

    mDragging = true;
    QPoint pos = QPoint(e->pos().x() - contentsRect().left(), e->pos().y() - contentsRect().top());

    if(pos.x() < 0)
      pos.setX(0);
    else if(pos.x() >= contentsRect().width())
      pos.setX(contentsRect().width()-1);

    if(pos.y() < 0)
      pos.setY(0);
    else if(pos.y() >= contentsRect().height())
      pos.setY(contentsRect().height()-1);

    QColor c = colorAt(pos);
    emit colorSelected(c);
  }
  else
    Q3Frame::mousePressEvent(e);
}

void KoColorFrame::mouseReleaseEvent(QMouseEvent *e)
{
  if(e->button() & Qt::LeftButton)
    mDragging = false;
  else
    Q3Frame::mouseReleaseEvent(e);
}

void KoColorFrame::mouseMoveEvent(QMouseEvent *e)
{
  if(mDragging)
  {
    bool set = false;
    int x = e->pos().x();
    int y = e->pos().y();

    int left = contentsRect().left();
    int right = contentsRect().left() + contentsRect().width();
    int top = contentsRect().top();
    int bottom =  contentsRect().top() + contentsRect().height();

    if(x < left)
    {
      x = left;
      set = true;
    }
    else if(x > right)
    {
      x = right;
      set = true;
    }
    if(y < top)
    {
      y = top;
      set = true;
    }
    else if(y > bottom)
    {
      y = bottom;
      set = true;
    }

//    if(set)
//      QCursor::setPos(mapToGlobal(QPoint(x,y)));

    QPoint pos = QPoint(x - contentsRect().left(), y - contentsRect().top());

    QColor c = colorAt(pos);
    emit colorSelected(c);
  }
  else
    Q3Frame::mouseMoveEvent(e);
}

/***********************************************************************************/

KoSliderWidget::KoSliderWidget(QWidget *parent):
QWidget(parent)
{
  mDragging = false;
  setFixedHeight(6);
  setFixedWidth(11);
}

void KoSliderWidget::paintEvent(QPaintEvent *)
{
  QPainter p;
  QPen pen(Qt::black, 1);
  p.begin(this);

  p.setPen(pen);
  p.drawLine(0, 5, 5, 0);
  p.drawLine(10, 5, 5, 0);
  p.drawLine(0, 5, 10, 5);
  p.end();
}

void KoSliderWidget::mousePressEvent(QMouseEvent *e)
{
  if(e->button() & Qt::LeftButton)
  {
    mPos = e->pos();
    mDragging = true;
  }
  else
    QWidget::mousePressEvent(e);
}

void KoSliderWidget::mouseReleaseEvent(QMouseEvent *e)
{
  if(e->button() & Qt::LeftButton)
    mDragging = false;
  else
    QWidget::mouseReleaseEvent(e);
}

void KoSliderWidget::mouseMoveEvent(QMouseEvent *e)
{
  if(mDragging)
  {
    QWidget *p = parentWidget();

    if(!p)
      return;

    QPoint newPos = p->mapFromGlobal(QCursor::pos()) - mPos;

    /* don't drag vertically */
    newPos.setY(pos().y());

    if(newPos.x() < 0)
      newPos.setX(0);
    if(newPos.x() > p->width() - width())
      newPos.setX(p->width() - width());

    move(newPos);
    emit positionChanged(newPos.x());
  }
  else
    QWidget::mouseMoveEvent(e);
}

/***********************************************************************************/

KoColorSlider::KoColorSlider(QWidget *parent):
QWidget(parent)
{
  mColorFrame = new KoColorFrame(this);
  mSlider = new KoSliderWidget(this);

  mMin = 0;
  mMax = 255;
  mValue = 0;

  connect(mSlider, SIGNAL(positionChanged(int)), this, SLOT(slotSliderMoved(int)));
  connect(mColorFrame, SIGNAL(clicked(const QPoint &)), this, SLOT(slotFrameClicked(const QPoint &)));
}

KoColorSlider::~KoColorSlider()
{
  delete mColorFrame;
  delete mSlider;
}

int KoColorSlider::minValue()
{
  return mMin;
}

int KoColorSlider::maxValue()
{
  return mMax;
}

void KoColorSlider::slotSetRange(int min, int max)
{
  if(min >= max)
    return;

  mMin = min;
  mMax = max;
}

void KoColorSlider::resizeEvent(QResizeEvent *e)
{
  QWidget::resizeEvent(e);
  // m_pSlider->width()/2 * 2 seems stupid but is not because for example
  // m_pSlider->width() == 11 I get 10.
  mColorFrame->setGeometry(mSlider->width() / 2, 0, width() - mSlider->width() / 2 * 2, height() - mSlider->height());
  slotSetValue(mValue);
}

void KoColorSlider::slotSetColor1(const QColor &c)
{
  mColorFrame->slotSetColor1(c);
}

void KoColorSlider::slotSetColor2(const QColor &c)
{
  mColorFrame->slotSetColor2(c);
}

void KoColorSlider::slotSetValue(int value)
{
  if(value < mMin)
    value = mMin;
  if(value > mMax)
    value = mMax;

  mValue = value;

  int range = mMax - mMin;
  float v = value;
  if(mMin < 0)
    v += -mMin;

  float factor = v / range;
  int x = static_cast<int>(factor * mColorFrame->contentsRect().width());

  mSlider->move(QPoint(x, height() - mSlider->height()));
}

void KoColorSlider::slotSliderMoved(int x)
{
  if(x < 0)
    x = 0;
  if(x > mColorFrame->contentsRect().width())
    x = mColorFrame->contentsRect().width();

  float factor = x;
  factor /= mColorFrame->contentsRect().width();
  int range = mMax - mMin;

  mValue = static_cast<int>(factor * range);

  emit valueChanged(mValue);
  emit colorSelected(mColorFrame->colorAt(QPoint(x, mColorFrame->contentsRect().height()/2)));
}

void KoColorSlider::slotFrameClicked(const QPoint &p)
{
  QPoint local = mColorFrame->mapToParent(p);
  QPoint pos = QPoint(local.x() - mSlider->width() / 2, height() - mSlider->height());

  if(pos.x() < 0)
    pos.setX(0);
  else if(pos.x() > width() - mSlider->width())
    pos.setX(width() - mSlider->width());

  mSlider->move(pos);
  slotSliderMoved(pos.x());
}

void KoColorSlider::mousePressEvent(QMouseEvent *e)
{
  if(e->button() & Qt::LeftButton)
  {
    QPoint pos = QPoint(e->pos().x() - mSlider->width() / 2, height() - mSlider->height());

    if(pos.x() < 0)
      pos.setX(0);
    else if(pos.x() > width() - mSlider->width())
      pos.setX(width() - mSlider->width());

    mSlider->move(pos);
    slotSliderMoved(pos.x());
  }
  else
    QWidget::mousePressEvent(e);
}

#include "koColorSlider.moc"
