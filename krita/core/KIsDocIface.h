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

#ifndef KIS_DOC_IFACE_H
#define KIS_DOC_IFACE_H

#include <KoViewIface.h>
#include <KoDocumentIface.h>
#include <dcopref.h>
#include <qstring.h>

class KisDoc;

class KIsDocIface : virtual public KoDocumentIface
{
	K_DCOP
		public:
	KIsDocIface( KisDoc *doc_ );
 k_dcop:
	virtual DCOPRef image( int num );

	virtual int undoLimit () const;
	virtual void setUndoLimit(int limit);
	virtual int redoLimit() const;
	virtual void setRedoLimit(int limit);

	virtual void renameImage(const QString& oldName, const QString& newName);

 private:
	KisDoc *m_doc;
};

#endif
