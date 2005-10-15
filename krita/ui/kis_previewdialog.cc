/*
 *  kis_previewdialog.cc - part of Krita
 *
 *  Copyright (c) 2005 Sven Langkamp <longamp@reallygood.de>
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
#include <qframe.h>
#include <qhbox.h>
#include <qgroupbox.h>
#include <qlayout.h>
 
#include "kis_previewwidget.h"
#include "kis_previewdialog.h"
 
KisPreviewDialog::KisPreviewDialog( QWidget *  parent, const char * name, bool modal, const QString &caption)
    : super (parent, name, modal, caption, Ok | Cancel, Ok)
{
    QHBox* layout = new QHBox(this);
    layout -> setSpacing( 6 );
    
    m_containerFrame = new QFrame( layout, "container" );

//     QGroupBox* groupBox = new QGroupBox( "Preview", layout, "groupBox" );
//     groupBox -> setColumnLayout(0, Qt::Vertical );
//     groupBox -> layout()->setSpacing( 6 );
//     groupBox -> layout()->setMargin( 11 );
//     QGridLayout *groupBoxLayout = new QGridLayout( groupBox -> layout(), 1, 1);
    m_preview = new KisPreviewWidget( layout/*groupBox*/, "previewWidget" );
//     groupBoxLayout -> addWidget( m_preview, 0 , 0);
    Q_CHECK_PTR(m_preview);

    setMainWidget(layout);
}

KisPreviewDialog::~KisPreviewDialog()
{

}

#include "kis_previewdialog.moc"
