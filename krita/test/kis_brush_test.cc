/*
 *  kis_brush_test.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qapplication.h>
#include <qwidget.h>
#include <qimage.h>
#include <qpainter.h>

#include <kis_brush.h>

class BrushWidget : public QWidget
{

public:
  BrushWidget( const QString& file, QWidget *parent=0, const char *name=0 );

protected:
  virtual void paintEvent ( QPaintEvent * );

private:
  KisBrush *b;
  QImage *i;
  int s;
};

BrushWidget::BrushWidget( const QString& file, QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  b = new KisBrush(file);
  i = new QImage(file);

  s = 3;

  resize (  3*s +  2 * b->width() , 3*s + 2 * b->height() );
  setMinimumSize(  3*s +  2 * b->width() , 3*s + 2 * b->height() );
}

void BrushWidget::paintEvent ( QPaintEvent * )
{
  QPainter p (this);

  // draw brush
  for (int y = 0; y < b->height(); y++)
    {
      for (int x = 0; x < b->width(); x++)
	{
	  uchar v = 255 - b->value(x, y);
	  p.setPen(QColor(v, v, v));
	  p.drawPoint(s + x, s + y);
	  p.drawPoint(2*s + x + b->width(), 2*s + y + b->height());
	}
    }
  // draw qimage
  p.drawImage( 2*s + b->width(), s, *i);
  p.drawImage( s , 2*s + b->height(), *i);
}

int main( int argc, char **argv )
{
  QApplication a( argc, argv );

  BrushWidget w (argv[1]);

  a.setMainWidget( &w );
  w.show();

  return a.exec();
}

