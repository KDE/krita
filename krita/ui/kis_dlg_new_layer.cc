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
#include <qlineedit.h>

#include <klocale.h>
#include <qvbox.h>

#include "kis_dlg_new_layer.h"

NewLayerDialog::NewLayerDialog( QWidget *parent, const char *name )
    : KDialogBase ( parent, name, true, "", Ok | Cancel )
{
    setCaption( i18n("New Image") );
    QVBox *page = makeVBoxMainWidget();

    QGridLayout* grid = new QGridLayout( page, 2, 2);

    m_width = new QSpinBox( 1, MAXIMAGEWIDTH, 10, page );
    m_width->setValue( INITIALWIDTH );
    QLabel* wlabel = new QLabel( m_width, i18n("W&idth:"), page );

    grid->addWidget( wlabel, 0, 0 );
    grid->addWidget( m_width, 0, 1 );

    m_height = new QSpinBox( 1, MAXIMAGEHEIGHT, 10, page );
    m_height->setValue( INITIALHEIGHT );
    QLabel* hlabel = new QLabel( m_height, i18n("&Height:"), page );

    grid->addWidget( hlabel, 1, 0 );
    grid->addWidget( m_height, 1, 1 );
    resize( 1, 1 );
}

#include "kis_dlg_new_layer.moc"
