/* This file is part of the KDE project
 *   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>
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

#include "kis_doc_iface.h"
#include <kapplication.h>

#include "kis_doc.h"
#include "kis_image.h"

#include <dcopclient.h>

KisDocIface::KisDocIface( KisDoc *doc_ )
    : KoDocumentIface( doc_ )
{
    m_doc = doc_;
}

DCOPRef  KisDocIface::currentImage()
{
    KisImage *img = m_doc->currentImage().data();
    if( !img )
        return DCOPRef();
    else
        return DCOPRef( kapp->dcopClient()->appId(),
                        img->dcopObject()->objId(),
                        "KisImageIface");
}

int KisDocIface::undoLimit () const
{
    return m_doc->undoLimit();
}

void KisDocIface::setUndoLimit(int limit)
{
    m_doc->setUndoLimit(limit);
}

int KisDocIface::redoLimit() const
{
    return m_doc->redoLimit();
}

void KisDocIface::setRedoLimit(int limit)
{
    m_doc->setRedoLimit(limit);
}

void KisDocIface::renameImage(const QString& oldName, const QString& newName)
{
    m_doc->renameImage(oldName,newName);
}
