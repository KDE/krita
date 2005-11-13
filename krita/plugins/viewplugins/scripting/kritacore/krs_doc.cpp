/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#include "krs_doc.h"

#include <kis_doc.h>
#include <kis_image.h>

#include "krs_image.h"

namespace Kross { namespace KritaCore {

KrsDoc::KrsDoc(::KisDoc* doc) : Kross::Api::Class<KrsDoc>("KritaDocument", 0 ), m_doc(doc) {
    addFunction("getImage", &KrsDoc::getImage);
}

KrsDoc::~KrsDoc() {
    
}

const QString KrsDoc::getClassName() const {
    return "Kross::KritaCore::KrsDoc";
}

Kross::Api::Object::Ptr KrsDoc::getImage(Kross::Api::List::Ptr)
{
    return new KrsImage(m_doc->currentImage());
}


}
}
