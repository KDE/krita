/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPADisplayMasterBackgroundCommand.h"

#include <klocalizedstring.h>

#include "KoPAPage.h"

KoPADisplayMasterBackgroundCommand::KoPADisplayMasterBackgroundCommand( KoPAPage * page, bool display )
: m_page( page )
, m_display( display )
{
    if ( m_display ) {
        setText( kundo2_i18n( "Display master background" ) );
    }
    else {
        if ( m_page->pageType() == KoPageApp::Slide ) {
            setText( kundo2_i18n( "Display slide background" ) );
        }
        else {
            setText( kundo2_i18n( "Display page background" ) );
        }
    }
}

KoPADisplayMasterBackgroundCommand::~KoPADisplayMasterBackgroundCommand()
{
}

void KoPADisplayMasterBackgroundCommand::redo()
{
    m_page->setDisplayMasterBackground( m_display );
    m_page->update();
}

void KoPADisplayMasterBackgroundCommand::undo()
{
    m_page->setDisplayMasterBackground( !m_display );
    m_page->update();
}
