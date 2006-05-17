/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000 theKompany.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "tkcombobox.h"

#include <q3listbox.h>
#include <QPainter>
#include <QStyle>
#include <qdrawutil.h>
#include <QStyleOption>
//Added by qt3to4:
#include <QPixmap>
#include <QPaintEvent>

#include <kapplication.h>

TKComboBox::TKComboBox(QWidget* parent, const char* name)
: Q3ComboBox(false,parent,name)
{
}


TKComboBox::TKComboBox( bool isEditable, QWidget* parent, const char* name )
: Q3ComboBox(isEditable,parent,name)
{
}

TKComboBox::~TKComboBox()
{
}

void TKComboBox::paintEvent(QPaintEvent*)
{
  QRect r;
  if (editable()){
#ifdef __GNUC__
#warning "Left out for now, lacking a style expert (Werner)"
#endif
    //r = QRect( style().comboButtonRect( 0, 0, width(), height() ) );
    r = QRect(4, 2, width()-height()-2, height()-4);
  } else {
    r = QRect(4, 2, width()-height()-2, height()-4);
  }
  int by = 2;
  int bx = r.x() + r.width();
  int bw = width() - bx - 2;
  int bh = height()-4;

  QPainter p( this );
  const QPalette& g = palette();

  QRect fr(2,2,width()-4,height()-4);

  if ( hasFocus()) {
    p.fillRect( fr, g.brush( QPalette::Highlight ) );
  } else {
    p.fillRect( fr, g.brush( QPalette::Base ) );
  }

  QRect r1(1,1,width()-1,height()-1);
  qDrawShadePanel( &p, r1, g, true, 1 );

  static const char* arrow_down[] = {
  "7 7 2 1",
  "X c Gray0",
  "  c None",
  "XXXXXXX",
  "XXXXXXX",
  "       ",
  "XXXXXXX",
  " XXXXX ",
  "  XXX  ",
  "   X   "};

  QPixmap pixmap(arrow_down);

  QStyleOption styleOption(QStyleOption::Version, QStyleOption::SO_Button);
  styleOption.rect = QRect(bx, by, bw, bh);
  style()->drawControl( QStyle::CE_PushButton, &styleOption, &p, this);
  style()->drawItemPixmap( &p, QRect( bx, by, bw, bh), Qt::AlignCenter, pixmap );

  if ( hasFocus()) {
    styleOption.type = QStyleOption::SO_FocusRect;
    styleOption.rect = fr;
    style()->drawPrimitive( QStyle::PE_FrameFocusRect, &styleOption, &p );
  }

  if (!editable()) {
    p.setClipRect(r);
    p.setPen( g.color(QPalette::Text ) );
    p.setBackground( g.background() );

    if ( listBox()->item(currentItem()) ) {
      Q3ListBoxItem * item = listBox()->item(currentItem());
      const QPixmap *pix = item->pixmap();
      QString text = item->text();
      int x = r.x();
      if ( pix ) {
        p.drawPixmap( x, r.y() + ( r.height() - pix->height() ) / 2 +1, *pix );
        x += pix->width()+3;
      }
      if (!text.isEmpty())
        p.drawText( x, r.y(), r.width()-x, r.height(), Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, text );
    }
  }
  p.end();
}

void TKComboBox::activate()
{
  emit activated(currentItem());
}

#include "tkcombobox.moc"
