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

#ifndef KOODF_H
#define KOODF_H

#include <koodf_export.h>

class KOODF_EXPORT KoOdf
{
public:
    enum DocumentType {
        Text,
        Graphics,
        Presentation,
        Spreadsheet,
        Chart,
        Image,
        Formula
    };

    static const char * mimeType( DocumentType documentType );
    static const char * templateMimeType( DocumentType documentType );
    static const char * bodyContentElement( DocumentType documentType, bool withNamespace );

private:
    struct DocumentData {
        const char * mimeType;
        const char * templateMimeType;
        const char * bodyContentElement;
    };

    static DocumentData sm_documentData[];
};

#endif /* KOODF_H */
