/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "KIsDocIface.h"
#include <kapplication.h>

#include "kis_doc.h"
#include <dcopclient.h>

KIsDocIface::KIsDocIface( KisDoc *doc_ )
	: KoDocumentIface( doc_ )
{
	m_doc = doc_;
}

DCOPRef KIsDocIface::image( int num )
{
	KisImage *img=m_doc->imageNum(num);
	if( !img )
		return DCOPRef();
	else
		return DCOPRef( kapp->dcopClient()->appId(),
				img->dcopObject()->objId() );

}

int KIsDocIface::undoLimit () const
{
	return m_doc->undoLimit();
}

void KIsDocIface::setUndoLimit(int limit)
{
	m_doc->setUndoLimit(limit);
}

int KIsDocIface::redoLimit() const
{
	return m_doc->redoLimit();
}

void KIsDocIface::setRedoLimit(int limit)
{
	m_doc->setRedoLimit(limit);
}

void KIsDocIface::renameImage(const QString& oldName, const QString& newName)
{
	m_doc->renameImage(oldName,newName);
}
