/* This file is part of the KDE project
 *  Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>
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

#ifndef KIS_DOC_IFACE_H
#define KIS_DOC_IFACE_H

#include <KoViewIface.h>
#include <KoDocumentIface.h>

#include <dcopref.h>
#include <QString>
#include <kis_image_iface.h>

class KisDoc;

class KisDocIface : virtual public KoDocumentIface
{
    K_DCOP
public:
    KisDocIface( KisDoc *doc_ );
k_dcop:
    virtual DCOPRef currentImage();

    virtual int undoLimit () const;
    virtual void setUndoLimit(int limit);
    virtual int redoLimit() const;
    virtual void setRedoLimit(int limit);

    virtual void renameImage(const QString& oldName, const QString& newName);

private:
    KisDoc *m_doc;
};

#endif
