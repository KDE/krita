/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KROSS_KRS_DOC_H_
#define _KROSS_KRS_DOC_H_

class KisDoc;

#include <api/class.h>

namespace Kross { namespace KritaCore {

class Doc : public Kross::Api::Class<Doc>
{
    public:
        explicit Doc(::KisDoc* doc);
        virtual ~Doc();
        virtual const QString getClassName() const;
    private:
        /**
         * This function return the Image associated with this Doc.
         * 
         * Example (in Ruby) :
         * @code
         * doc = krosskritacore::get("KritaDocument")
         * image = doc.getImage()
         * @endcode
         */
        Kross::Api::Object::Ptr getImage(Kross::Api::List::Ptr);
    private:
        KisDoc* m_doc;

};
}
}


#endif
