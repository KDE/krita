/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or ( at your option ) any later version.
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

#include "KoPAChangeMasterPageCommand.h"

#include <klocale.h>

#include "KoPAPage.h";

KoPAChangeMasterPageCommand::KoPAChangeMasterPageCommand( KoPAPage * page, KoPAMasterPage * masterPage )
: m_page( page )
, m_oldMasterPage( page->masterPage() )
, m_newMasterPage( masterPage )
{
    setText( i18n( "Change master page" ) );
}

KoPAChangeMasterPageCommand::~KoPAChangeMasterPageCommand()
{
}

void KoPAChangeMasterPageCommand::redo()
{
    m_page->setMasterPage( m_newMasterPage );
    // TODO add a way to update all views that show m_page
}

void KoPAChangeMasterPageCommand::undo()
{
    m_page->setMasterPage( m_oldMasterPage );
    // TODO add a way to update all views that show m_page
}
