/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2010 Thomas Zander <zander@kde.org>

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

#include "KoOdf.h"

namespace KoOdf
{
struct DocumentData {
    const char * mimeType;
    const char * templateMimeType;
    const char * bodyContentElement;
};

DocumentData s_documentData[] = {
    { "application/vnd.oasis.opendocument.text", "application/vnd.oasis.opendocument.text-template", "office:text" },
    { "application/vnd.oasis.opendocument.graphics", "application/vnd.oasis.opendocument.graphics-template", "office:drawing" },
    { "application/vnd.oasis.opendocument.presentation", "application/vnd.oasis.opendocument.presentation-template", "office:presentation" },
    { "application/vnd.oasis.opendocument.spreadsheet", "application/vnd.oasis.opendocument.spreadsheet-template", "office:spreadsheet" },
    { "application/vnd.oasis.opendocument.chart", "application/vnd.oasis.opendocument.chart-template", "office:chart" },
    { "application/vnd.oasis.opendocument.image", "application/vnd.oasis.opendocument.image-template", "office:image" },
    // TODO what is the element for a formula check if bodyContentElement is ok
    { "application/vnd.oasis.opendocument.formula", "application/vnd.oasis.opendocument.formula-template", "office:XXX" }
};
//"application/vnd.oasis.opendocument.text-master"
//"application/vnd.oasis.opendocument.text-web"

const char * mimeType(DocumentType documentType)
{
    return s_documentData[documentType].mimeType;
}

const char * templateMimeType(DocumentType documentType)
{
    return s_documentData[documentType].templateMimeType;
}

const char * bodyContentElement(DocumentType documentType, bool withNamespace)
{
    return withNamespace ? s_documentData[documentType].bodyContentElement : s_documentData[documentType].bodyContentElement + 7;
}

}
