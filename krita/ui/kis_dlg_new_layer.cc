/*
 *  newlayerdialog.cc - part of KImageShop
 *
 *  Copyright (c) 2000 Michael Koch <koch@kde.org>
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

// defined in kis_dlg_new.cc

#ifndef KDE_USE_FINAL 
const int MAXIMAGEWIDTH = 32767;
const int INITIALWIDTH = 512;
const int MAXIMAGEHEIGHT = 32767;
const int INITIALHEIGHT = 512;
#endif

#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qspinbox.h>

#include <klocale.h>

#include "kis_dlg_new_layer.h"

NewLayerDialog::NewLayerDialog( QWidget *parent, const char *name )
    : KDialog( parent, name, true )
{
    setCaption( i18n("New Image") );

    QVBoxLayout* layout = new QVBoxLayout( this, 3 );

    QGridLayout* grid = new QGridLayout( layout, 2, 2);

    m_width = new QSpinBox( 1, MAXIMAGEWIDTH, 10, this );
    m_width->setValue( INITIALWIDTH );
    QLabel* wlabel = new QLabel( m_width, i18n("W&idth"), this );

    grid->addWidget( wlabel, 0, 0 );
    grid->addWidget( m_width, 0, 1 );

    m_height = new QSpinBox( 1, MAXIMAGEHEIGHT, 10, this );
    m_height->setValue( INITIALHEIGHT );
    QLabel* hlabel = new QLabel( m_height, i18n("&Height"), this );

    grid->addWidget( hlabel, 1, 0 );
    grid->addWidget( m_height, 1, 1 );

    QHBoxLayout* buttons = new QHBoxLayout( layout );

    buttons->addStretch( 1 );

    QPushButton *ok, *cancel;
    ok = new QPushButton( i18n("&OK"), this );
    ok->setDefault( true );
    ok->setMinimumSize( ok->sizeHint() );
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );
    buttons->addWidget( ok );

    cancel = new QPushButton( i18n("&Cancel"), this );
    cancel->setMinimumSize( cancel->sizeHint() );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
    buttons->addWidget( cancel );

    resize( 1, 1 );
}

#include "kis_dlg_new_layer.moc"
