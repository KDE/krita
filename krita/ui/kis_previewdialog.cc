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
#include <QFrame>
#include <QLayout>

#include <khbox.h>

#include "kis_previewwidget.h"
#include "kis_previewdialog.h"

KisPreviewDialog::KisPreviewDialog(QWidget *  parent, const char * name, const QString caption)
    : super(parent, caption, Ok | Cancel)
{
    setObjectName(name);
    KHBox* layout = new KHBox(this);
    layout->setSpacing( 6 );

    m_containerFrame = new QFrame( layout );
    m_containerFrame->setObjectName("container");

    m_preview = new KisPreviewWidget( layout, "previewWidget" );

    setMainWidget(layout);
}

KisPreviewDialog::~KisPreviewDialog()
{

}

#include "kis_previewdialog.moc"
