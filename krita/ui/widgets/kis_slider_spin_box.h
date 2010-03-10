/* This file is part of the KDE project
 * Copyright (c) 2010 Justin Noel <justin@ics.com>
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
#ifndef KISSLIDERSPINBOX_H
#define KISSLIDERSPINBOX_H

#include <QAbstractSlider>

#include <QStyleOptionSpinBox>
#include <QStyleOptionProgressBar>
#include <krita_export.h>

class QLineEdit;
class QDoubleValidator;
class QTimer;

class  KRITAUI_EXPORT KisSliderSpinBox : public QAbstractSlider
{
   Q_OBJECT
   Q_DISABLE_COPY(KisSliderSpinBox)
public:
   explicit KisSliderSpinBox(QWidget* parent=0);
   virtual ~KisSliderSpinBox();

   void showEdit();
   void hideEdit();

   void setRange(qreal minimum, qreal maximum, int decimals = 0);
   
   void setSuffix(const QString& suffix);
   
   ///Get the value, don't use value()
   qreal doubleValue();

   ///Set the value, don't use setValue()
   void setDoubleValue(qreal value);

   void setExponentRatio(qreal dbl);   
protected:
   virtual void paintEvent(QPaintEvent* e);
   virtual void mousePressEvent(QMouseEvent* e);
   virtual void mouseReleaseEvent(QMouseEvent* e);
   virtual void mouseMoveEvent(QMouseEvent* e);
   virtual void mouseDoubleClickEvent(QMouseEvent* e);
   virtual void keyPressEvent(QKeyEvent* e);

   virtual bool eventFilter(QObject* recv, QEvent* e);

   virtual QSize sizeHint() const;
   virtual QSize minimumSizeHint() const;

   QStyleOptionSpinBox spinBoxOptions() const;
   QStyleOptionProgressBar progressBarOptions() const;

   QRect progressRect(const QStyleOptionSpinBox& spinBoxOptions) const;
   QRect upButtonRect(const QStyleOptionSpinBox& spinBoxOptions) const;
   QRect downButtonRect(const QStyleOptionSpinBox& spinBoxOptions) const;

   int valueForX(int x) const;
   
   QString valueString() const;
signals:
    void doubleValueChanged(qreal value);
   
protected slots:
   void contextMenuEvent(QContextMenuEvent * event);
   void internalValueChanged(int value);

private:
    struct Private;
    Private* const d;
};

#endif //kISSLIDERSPINBOX_H
