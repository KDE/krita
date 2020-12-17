/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2004 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOXMLNS_H
#define KOXMLNS_H

#include <QString>

#include "kritastore_export.h"
/**
 * Repository of XML namespaces used for ODF documents.
 * 
 * Please make sure that you do not use the variables provided by this class in
 * the destructor of a static object.
 */
class KRITASTORE_EXPORT KoXmlNS
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
    static const QString sodipodi;
    static const QString krita;

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
