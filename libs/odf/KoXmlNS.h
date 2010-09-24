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

#ifndef KOXMLNS_H
#define KOXMLNS_H

#include "koodf_export.h"
/**
 * Repository of XML namespaces used for OASIS documents.
 */
class KOODF_EXPORT KoXmlNS
{
public:
    static const char* const office;
    static const char* const meta;
    static const char* const config;
    static const char* const text;
    static const char* const table;
    static const char* const draw;
    static const char* const presentation;
    static const char* const dr3d;
    static const char* const chart;
    static const char* const form;
    static const char* const script;
    static const char* const style;
    static const char* const number;
    static const char* const manifest;
    static const char* const anim;

    static const char* const math;
    static const char* const svg;
    static const char* const fo;
    static const char* const dc;
    static const char* const xlink;
    static const char* const VL;
    static const char* const smil;
    static const char* const xhtml;

    static const char* const koffice;
    static const char* const officeooo;

    static const char* nsURI2NS(const char* nsURI);
private:
    KoXmlNS(); // don't create an instance of me :)
};

#endif /* KOXMLNS_H */
