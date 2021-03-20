/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2005-2006 Ariya Hidayat <ariya@kde.org>
   SPDX-FileCopyrightText: 2007 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOXMLREADERFORWARD_H
#define KOXMLREADERFORWARD_H

// use standard QDom, useful to test KoXml classes against Qt's QDom
#include <QDomDocument>

typedef QDomNode KoXmlNode;
typedef QDomElement KoXmlElement;
typedef QDomText KoXmlText;
typedef QDomCDATASection KoXmlCDATASection;
typedef QDomDocumentType KoXmlDocumentType;
typedef QDomDocument KoXmlDocument;


#endif // KOXMLREADERFORWARD_H
