/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

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

#include "KoXmlNS.h"

#include <string.h>

const char* const KoXmlNS::office = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
const char* const KoXmlNS::meta = "urn:oasis:names:tc:opendocument:xmlns:meta:1.0";
const char* const KoXmlNS::config = "urn:oasis:names:tc:opendocument:xmlns:config:1.0";
const char* const KoXmlNS::text = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
const char* const KoXmlNS::table = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
const char* const KoXmlNS::draw = "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0";
const char* const KoXmlNS::presentation = "urn:oasis:names:tc:opendocument:xmlns:presentation:1.0";
const char* const KoXmlNS::dr3d = "urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0";
const char* const KoXmlNS::chart = "urn:oasis:names:tc:opendocument:xmlns:chart:1.0";
const char* const KoXmlNS::form = "urn:oasis:names:tc:opendocument:xmlns:form:1.0";
const char* const KoXmlNS::script = "urn:oasis:names:tc:opendocument:xmlns:script:1.0";
const char* const KoXmlNS::style = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";
const char* const KoXmlNS::number = "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0";
const char* const KoXmlNS::manifest = "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0";
const char* const KoXmlNS::anim = "urn:oasis:names:tc:opendocument:xmlns:animation:1.0";

const char* const KoXmlNS::math = "http://www.w3.org/1998/Math/MathML";
const char* const KoXmlNS::svg = "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0";
const char* const KoXmlNS::fo = "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0";
const char* const KoXmlNS::dc = "http://purl.org/dc/elements/1.1/";
const char* const KoXmlNS::xlink = "http://www.w3.org/1999/xlink";
const char* const KoXmlNS::VL = "http://openoffice.org/2001/versions-list";
const char* const KoXmlNS::smil = "urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0";
const char* const KoXmlNS::xhtml = "http://www.w3.org/1999/xhtml";

const char* const KoXmlNS::koffice = "http://www.koffice.org/2005/";
const char* const KoXmlNS::officeooo = "http://openoffice.org/2009/office";

const char* KoXmlNS::nsURI2NS(const char* nsURI)
{
    if (strcmp(nsURI, KoXmlNS::office) == 0)
        return "office";
    else if (strcmp(nsURI, KoXmlNS::meta) == 0)
        return "meta";
    else if (strcmp(nsURI, KoXmlNS::config) == 0)
        return "config";
    else if (strcmp(nsURI, KoXmlNS::text) == 0)
        return "text";
    else if (strcmp(nsURI, KoXmlNS::table) == 0)
        return "table";
    else if (strcmp(nsURI, KoXmlNS::draw) == 0)
        return "draw";
    else if (strcmp(nsURI, KoXmlNS::presentation) == 0)
        return "presentation";
    else if (strcmp(nsURI, KoXmlNS::dr3d) == 0)
        return "dr3d";
    else if (strcmp(nsURI, KoXmlNS::chart) == 0)
        return "chart";
    else if (strcmp(nsURI, KoXmlNS::form) == 0)
        return "form";
    else if (strcmp(nsURI, KoXmlNS::script) == 0)
        return "script";
    else if (strcmp(nsURI, KoXmlNS::style) == 0)
        return "style";
    else if (strcmp(nsURI, KoXmlNS::number) == 0)
        return "number";
    else if (strcmp(nsURI, KoXmlNS::manifest) == 0)
        return "manifest";
    else if (strcmp(nsURI, KoXmlNS::anim) == 0)
        return "anim";
    else if (strcmp(nsURI, KoXmlNS::math) == 0)
        return "math";
    else if (strcmp(nsURI, KoXmlNS::svg) == 0)
        return "svg";
    else if (strcmp(nsURI, KoXmlNS::fo) == 0)
        return "fo";
    else if (strcmp(nsURI, KoXmlNS::dc) == 0)
        return "dc";
    else if (strcmp(nsURI, KoXmlNS::xlink) == 0)
        return "xlink";
    else if (strcmp(nsURI, KoXmlNS::VL) == 0)
        return "VL";
    else if (strcmp(nsURI, KoXmlNS::smil) == 0)
        return "smil";
    else if (strcmp(nsURI, KoXmlNS::xhtml) == 0)
        return "xhtml";
    else if (strcmp(nsURI, KoXmlNS::koffice) == 0)
        return "koffice";
    else if (strcmp(nsURI, KoXmlNS::officeooo) == 0)
        return "officeooo";

    // Shouldn't happen.
    return "";
}
