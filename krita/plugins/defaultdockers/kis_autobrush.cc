/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 
#include "kis_autobrush.h"
#include <kdebug.h>
#include <qspinbox.h>
#include <qtoolbutton.h>
#include <qimage.h>
#include <qcombobox.h>
#include <qlabel.h>


namespace {
    /* XPM -- copyright The Gimp */
    const char *chain_broken_24[] = {
    /* columns rows colors chars-per-pixel */
    "9 24 10 1",
    "  c black",
    ". c #020204",
    "X c #5A5A5C",
    "o c gray43",
    "O c #8F8F91",
    "+ c #9A9A98",
    "@ c #B5B5B6",
    "# c #D0D0D1",
    "$ c #E8E8E9",
    "% c None",
    /* pixels */
    "%%.....%%",
    "%.o##@X.%",
    "%.+...$.%",
    "%.#.%.#.%",
    "%.#.%.#.%",
    "%.@.%.#.%",
    "%.+...#.%",
    "%.O.o.O.%",
    "%%..@..%%",
    "%%%.#.%%%",
    "%%%%%%%%%",
    "%%%%%%%%%",
    "%%%%%%%%%",
    "%%%%%%%%%",
    "%%%.#.%%%",
    "%%..#..%%",
    "%.o.@.O.%",
    "%.@...@.%",
    "%.@.%.$.%",
    "%.@.%.$.%",
    "%.@.%.$.%",
    "%.#...$.%",
    "%.o$#$@.%",
    "%%.....%%"
    };
    
    /* XPM  -- copyright The Gimp */
    const char *chain_24[] = {
    /* columns rows colors chars-per-pixel */
    "9 24 10 1",
    "  c black",
    ". c #020204",
    "X c #5A5A5C",
    "o c gray43",
    "O c #8F8F91",
    "+ c #9A9A98",
    "@ c #B5B5B6",
    "# c #D0D0D1",
    "$ c #E8E8E9",
    "% c None",
    /* pixels */
    "%%%%%%%%%",
    "%%%%%%%%%",
    "%%.....%%",
    "%.o##@X.%",
    "%.+...$.%",
    "%.#.%.#.%",
    "%.#.%.#.%",
    "%.@.%.#.%",
    "%.+...#.%",
    "%.O.o.O.%",
    "%%..@..%%",
    "%%%.#.%%%",
    "%%%.#.%%%",
    "%%..#..%%",
    "%.o.@.O.%",
    "%.@...@.%",
    "%.@.%.$.%",
    "%.@.%.$.%",
    "%.@.%.$.%",
    "%.#...$.%",
    "%.o$#$@.%",
    "%%.....%%",
    "%%%%%%%%%",
    "%%%%%%%%%"
    };


}

KisAutobrush::KisAutobrush(QWidget *parent, const char* name, const QString& caption) : KisWdgAutobrush(parent, name)
{
    setCaption(caption);

    m_linkSize = true;
    m_linkFade = true;
    
    linkFadeToggled(m_linkSize);
    linkSizeToggled(m_linkFade);

    connect(bnLinkSize, SIGNAL(toggled(bool)), this, SLOT(linkSizeToggled( bool )));
    connect(bnLinkFade, SIGNAL(toggled(bool)), this, SLOT(linkFadeToggled( bool )));
    
    connect((QObject*)comboBoxShape, SIGNAL(activated(int)), this, SLOT(paramChanged()));
    spinBoxWidth->setMinValue(1);
    connect(spinBoxWidth,SIGNAL(valueChanged(int)),this,SLOT(spinBoxWidthChanged(int)));
    spinBoxHeigth->setMinValue(1);
    connect(spinBoxHeigth,SIGNAL(valueChanged(int)),this,SLOT(spinBoxHeigthChanged(int)));
    spinBoxHorizontal->setMinValue(0);
    connect(spinBoxHorizontal,SIGNAL(valueChanged(int)),this,SLOT(spinBoxHorizontalChanged(int)));
    spinBoxVertical->setMinValue(0);
    connect(spinBoxVertical,SIGNAL(valueChanged(int)),this,SLOT(spinBoxVerticalChanged(int)));

    m_brsh = new QImage(1,1,32);
    Q_CHECK_PTR(m_brsh);

    paramChanged();

    
    connect(brushPreview, SIGNAL(clicked()), SLOT(paramChanged()));

}

void KisAutobrush::activate()
{
    paramChanged();
}

void KisAutobrush::paramChanged()
{
    Q_INT32 fh = QMIN( spinBoxWidth->value()/2, spinBoxHorizontal->value() ) ;
    Q_INT32 fv = QMIN( spinBoxHeigth->value()/2, spinBoxVertical->value() );
    KisAutobrushShape* kas;

    if(comboBoxShape->currentItem() == 0) // use index compare instead of comparing a translatable string
    {
        kas = new KisAutobrushCircleShape(spinBoxWidth->value(),  spinBoxHeigth->value(), fh, fv);
        Q_CHECK_PTR(kas);

    } else {
        kas = new KisAutobrushRectShape(spinBoxWidth->value(),  spinBoxHeigth->value(), fh, fv);
        Q_CHECK_PTR(kas);

    }
    kas->createBrush(m_brsh);

    QPixmap p;
    p.convertFromImage(*m_brsh);
    brushPreview->setPixmap(p);
    KisAutobrushResource * resource = new KisAutobrushResource(*m_brsh);
    Q_CHECK_PTR(resource);

    emit(activatedResource(resource));
    delete kas;
}
void KisAutobrush::spinBoxWidthChanged(int a)
{
    spinBoxHorizontal->setMaxValue(a/2);
    if(m_linkSize)
    {
        spinBoxHeigth->setValue(a);
        spinBoxVertical->setMaxValue(a/2);
    }
    this->paramChanged();
}
void KisAutobrush::spinBoxHeigthChanged(int a)
{
    spinBoxVertical->setMaxValue(a/2);
    if(m_linkSize)
    {
        spinBoxWidth->setValue(a);
        spinBoxHorizontal->setMaxValue(a/2);
    }
    this->paramChanged();
}
void KisAutobrush::spinBoxHorizontalChanged(int a)
{
    if(m_linkFade)
        spinBoxVertical->setValue(a);
    this->paramChanged();
}
void KisAutobrush::spinBoxVerticalChanged(int a)
{
    if(m_linkFade)
        spinBoxHorizontal->setValue(a);
    this->paramChanged();
}

void KisAutobrush::linkSizeToggled(bool b)
{
    m_linkSize = b;

    if (b) {
        bnLinkSize->setPixmap(chain_24);
    }
    else {
        bnLinkSize->setPixmap(chain_broken_24);
    }
}

void KisAutobrush::linkFadeToggled(bool b)
{
    m_linkFade = b;

    if (b) {
        bnLinkFade->setPixmap(chain_24);
    }
    else {
        bnLinkFade->setPixmap(chain_broken_24);
    }
}


#include "kis_autobrush.moc"
