/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

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

#ifndef __rdf_RdfForward_h__
#define __rdf_RdfForward_h__

#include <QExplicitlySharedDataPointer>

class KoDocumentRdf;
class KoDocumentRdfEditWidget;
class KoTextInlineRdf;
class KoRdfPrefixMapping;
class KoRdfSemanticTreeWidgetItem;
class KoRdfFoaFTreeWidgetItem;
class KoTextEditor;
class KoRdfSemanticItem;
class KoRdfFoaF;
class KoRdfCalendarEvent;
class KoRdfLocation;
class KoSemanticStylesheet;

namespace Ui
{
    class KoRdfLocationEditWidget;
}
// namespace Marble
// {
//     class MarbleWidget;
//     class LatLonEdit;
// }
namespace Soprano
{
    class Model;
    class Statement;
    class Node;
}
typedef QExplicitlySharedDataPointer<KoRdfSemanticItem> hKoRdfSemanticItem;
typedef QExplicitlySharedDataPointer<KoRdfFoaF> hKoRdfFoaF;
typedef QExplicitlySharedDataPointer<KoRdfCalendarEvent> hKoRdfCalendarEvent;
typedef QExplicitlySharedDataPointer<KoRdfLocation> hKoRdfLocation;
typedef QExplicitlySharedDataPointer<KoSemanticStylesheet> hKoSemanticStylesheet;

#endif
