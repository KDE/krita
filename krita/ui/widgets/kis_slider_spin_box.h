/* This file is part of the KDE project
 * Copyright 2010 (C) Justin Noel <justin@ics.com>
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

class QLineEdit;
class QIntValidator;
class QTimer;

class KisSliderSpinBox : public QAbstractSlider
{
   Q_OBJECT
public:
   explicit KisSliderSpinBox(QWidget* parent=0);
   virtual ~KisSliderSpinBox();

   void showEdit();
   void hideEdit();

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

protected slots:
   void updateValidatorRange(int min, int max);
   void contextMenuEvent(QContextMenuEvent * event); 

private:
   QLineEdit* m_edit;
   QIntValidator* m_validator;
   bool m_upButtonDown;
   bool m_downButtonDown;

};

#endif //kISSLIDERSPINBOX_H
