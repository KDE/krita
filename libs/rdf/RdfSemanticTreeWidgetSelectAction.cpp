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

#include "RdfSemanticTreeWidgetSelectAction.h"

#include "KoDocumentRdf.h"
#include "KoRdfSemanticItemViewSite.h"

#include <KoTextEditor.h>

#include <kdebug.h>

RdfSemanticTreeWidgetSelectAction::RdfSemanticTreeWidgetSelectAction(QWidget *parent,
        KoCanvasBase *canvas, hKoRdfSemanticItem si, const QString &name)
    : RdfSemanticTreeWidgetAction(parent, canvas, name)
    , si(si)
{
    kDebug(30015) << "select action ctor";
}

RdfSemanticTreeWidgetSelectAction::~RdfSemanticTreeWidgetSelectAction()
{
}

void RdfSemanticTreeWidgetSelectAction::activated()
{
    const KoDocumentRdf *rdf = si->documentRdf();
    QString xmlid = rdf->findXmlId(KoTextEditor::getTextEditorFromCanvas(m_canvas));
    kDebug(30015) << "selecting xmlid:" << xmlid;
    KoRdfSemanticItemViewSite vs(si, xmlid);
    vs.select(m_canvas);
}

