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

#include "koodf_export.h"

namespace KoOdf
{
    enum DocumentType {
        Text,
        Graphics,
        Presentation,
        Spreadsheet,
        Chart,
        Image,
        Formula
    };

    /**
     * Get the mime type
     *
     * @param documentType the document type
     * @return the mime type used for the given document type
     */
    KOODF_EXPORT const char * mimeType(DocumentType documentType);

    /**
     * Get the mime type
     *
     * @param documentType the document type
     * @return the mime type used for templates of the given document type
     */
    KOODF_EXPORT const char * templateMimeType(DocumentType documentType);

    /**
     * Get the mime type
     *
     * @param documentType the document type
     * @param withNamespace if true the namespace before the element is also returned
     *                      if false only the element is returned
     * @return the body element name for the given document type
     */
    KOODF_EXPORT const char * bodyContentElement(DocumentType documentType, bool withNamespace);
}

#endif /* KOODF_H */
