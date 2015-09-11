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

#include "InsertSemanticObjectCreateAction.h"
#include "KoDocumentRdf.h"

#include "KoCanvasBase.h"
#include "KoToolProxy.h"
#include "KoTextEditor.h"

#include <kdebug.h>
#include <klocalizedstring.h>
#include <kpagedialog.h>

#include <QVBoxLayout>

InsertSemanticObjectCreateAction::InsertSemanticObjectCreateAction(
    KoCanvasBase *canvas,
    KoDocumentRdf *rdf,
    const QString &name)
        : InsertSemanticObjectActionBase(canvas, rdf, name),
        m_semanticClass(name)
{
}

InsertSemanticObjectCreateAction::~InsertSemanticObjectCreateAction()
{
}

void InsertSemanticObjectCreateAction::activated()
{
    kDebug(30015) << "create semantic action...";
    QWidget *widget = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(widget);
    widget->setLayout(lay);
    lay->setMargin(0);
    kDebug(30015) << "semanticClass:" << m_semanticClass;
    hKoRdfSemanticItem semItem(static_cast<KoRdfSemanticItem *>(
	m_rdf->createSemanticItem(m_semanticClass, m_rdf).data()));
    QWidget *w = semItem->createEditor(widget);
    lay->addWidget(w);
    KPageDialog dialog(m_canvas->canvasWidget());
    dialog.setWindowTitle(i18n("%1 Options", text().remove('&'))); // TODO add comment using i18nc
    dialog.addPage(widget, QString());
    if (dialog.exec() == KPageDialog::Accepted) {
        kDebug(30015) << "activated...";
        semItem->updateFromEditorData();
        semItem->insert(m_canvas);
    }
}
