// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C)  2002 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2005 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoGuideLineDia.h"

#include <q3buttongroup.h>


#include <QLabel>
#include <QLayout>
#include <QRadioButton>

#include <klocale.h>
#include <KoUnitWidgets.h>
#include <kvbox.h>


KoGuideLineDia::KoGuideLineDia( QWidget *parent, double pos, double minPos, double maxPos,
                                KoUnit::Unit unit, const char *name )
: KDialogBase( parent, name , true, "", Ok | Cancel, Ok, true )
, m_hButton( 0 )
, m_vButton( 0 )
{
    setCaption( i18n("Set Guide Line Position") );
    KHBox *page = makeHBoxMainWidget();
    new QLabel( i18n( "Position:" ), page );
    m_position= new KoUnitDoubleSpinBox( page, qMax( 0.00, minPos ), qMax( 0.00, maxPos ), 1, qMax( 0.00, pos ), unit );
    m_position->setFocus();
}


KoGuideLineDia::KoGuideLineDia( QWidget *parent, KoPoint &pos, KoRect &rect,
                                KoUnit::Unit unit, const char *name )
: KDialogBase( parent, name , true, "", Ok | Cancel, Ok, true )
, m_rect( rect )
, m_pos( pos )
, m_positionChanged( false )
, m_hButton( 0 )
, m_vButton( 0 )
{
    setCaption( i18n("Add Guide Line") );
    KVBox * vbox = makeVBoxMainWidget();

    Q3ButtonGroup *group = new Q3ButtonGroup( 1, Qt::Horizontal, i18n( "Orientation" ), vbox );
    group->setRadioButtonExclusive( true );
    //group->layout();
    m_hButton = new QRadioButton( i18n( "Horizontal" ), group );
    m_vButton = new QRadioButton( i18n( "Vertical" ), group );

    connect( group, SIGNAL( clicked( int ) ), this, SLOT( slotOrientationChanged() ) );

    m_vButton->setChecked( true );;

    KHBox *hbox = new KHBox( vbox );
    QLabel *label = new QLabel( i18n( "&Position:" ), hbox );
    m_position= new KoUnitDoubleSpinBox( hbox, qMax( 0.0, m_rect.left() ), qMax( 0.0, m_rect.right() ), 1, qMax( 0.0, pos.x() ), unit );
    m_position->setFocus();
    label->setBuddy( m_position );

    connect( m_position, SIGNAL( valueChanged( double ) ), this, SLOT( slotPositionChanged() ) );
}


double KoGuideLineDia::pos() const
{
    return m_position->value();
}


Qt::Orientation KoGuideLineDia::orientation() const
{
    Qt::Orientation o = Qt::Horizontal;
    if ( m_vButton && m_vButton->isChecked() )
    {
        o = Qt::Vertical;
    }
    return o;
}


void KoGuideLineDia::slotOrientationChanged()
{
    if ( m_hButton && m_vButton )
    {
        if ( m_hButton->isChecked() )
        {
            m_position->setMinimum( qMax( 0.0, m_rect.top() ) );
            m_position->setMaximum( qMax( 0.0, m_rect.bottom() ) );
            if ( ! m_positionChanged )
            {
                disconnect( m_position, SIGNAL( valueChanged( double ) ), this, SLOT( slotPositionChanged() ) );
                m_position->changeValue( m_pos.y() );
                connect( m_position, SIGNAL( valueChanged( double ) ), this, SLOT( slotPositionChanged() ) );
            }
        }
        else if ( m_vButton->isChecked() )
        {
            m_position->setMinimum( qMax( 0.0, m_rect.left() ) );
            m_position->setMaximum( qMax( 0.0, m_rect.right() ) );
            if ( ! m_positionChanged )
            {
                disconnect( m_position, SIGNAL( valueChanged( double ) ), this, SLOT( slotPositionChanged() ) );
                m_position->changeValue( m_pos.x() );
                connect( m_position, SIGNAL( valueChanged( double ) ), this, SLOT( slotPositionChanged() ) );
            }
        }
    }
}

void KoGuideLineDia::slotPositionChanged()
{
    m_positionChanged = true;
}
#include "KoGuideLineDia.moc"
