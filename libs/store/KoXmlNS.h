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

#include <QString>

#include "kostore_export.h"
/**
 * Repository of XML namespaces used for ODF documents.
 * 
 * Please make sure that you do not use the variables provided by this class in
 * the destructor of a static object.
 */
class KOSTORE_EXPORT KoXmlNS
{
public:
    static const QString office;
    static const QString meta;
    static const QString config;
    static const QString text;
    static const QString table;
    static const QString draw;
    static const QString presentation;
    static const QString dr3d;
    static const QString chart;
    static const QString form;
    static const QString script;
    static const QString style;
    static const QString number;
    static const QString manifest;
    static const QString anim;

    static const QString math;
    static const QString svg;
    static const QString fo;
    static const QString dc;
    static const QString xlink;
    static const QString VL;
    static const QString smil;
    static const QString xhtml;
    static const QString xml;

    static const QString calligra;
    static const QString officeooo;
    static const QString ooo;

    static const char* nsURI2NS(const QString &nsURI);
    
    static const QString delta;
    static const QString split;
    static const QString ac;
private:
    KoXmlNS(); // don't create an instance of me :)
};

#endif /* KOXMLNS_H */
