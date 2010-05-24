/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOODFPASTE_H
#define KOODFPASTE_H

#include "KoOdf.h"
#include "koodf_export.h"
#include "KoXmlReaderForward.h"

class QMimeData;
class KoOdfReadStore;

/**
 * This is a helper class to help you paste odf snippets.
 */
class KOODF_EXPORT KoOdfPaste
{
public:
    KoOdfPaste();
    virtual ~KoOdfPaste();

    bool paste(KoOdf::DocumentType documentType, const QMimeData *data);
    /**
     * This is an overloaded member function, provided for convenience. It differs
     * from the above function only in what argument(s) it accepts.
     */
    bool paste(KoOdf::DocumentType documentType, const QByteArray &data);

protected:
    virtual bool process(const KoXmlElement &body, KoOdfReadStore &odfStore) = 0;
};

#endif /* KOODFPASTE_H */
