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

#include "InsertSemanticObjectReferenceAction.h"

#include "KoDocumentRdf.h"

#include <KoCanvasBase.h>
#include <KoToolProxy.h>
#include <KoTextEditor.h>
#include "KoRdfSemanticTreeWidgetItem.h"

#include <kdebug.h>
#include <klocalizedstring.h>
#include <kpagedialog.h>

#include <QVBoxLayout>
#include <QLabel>

InsertSemanticObjectReferenceAction::InsertSemanticObjectReferenceAction(
    KoCanvasBase *canvas,
    KoDocumentRdf *rdf,
    const QString &name)
        : InsertSemanticObjectActionBase(canvas, rdf, name)
{
}

InsertSemanticObjectReferenceAction::~InsertSemanticObjectReferenceAction()
{
}

void InsertSemanticObjectReferenceAction::activated()
{
    kDebug(30015) << "create semantic item reference...";
    QWidget *widget = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(widget);
    widget->setLayout(lay);
    lay->setMargin(0);

    QLabel *label = new QLabel(i18n("Select the object you want to reference"), widget);
    lay->addWidget(label);
    QTreeWidget *tree = new QTreeWidget(widget);
    KoRdfSemanticTree td = KoRdfSemanticTree::createTree(tree);
    td.update(const_cast<KoDocumentRdf*>(m_rdf));
    lay->addWidget(tree);

    KPageDialog dialog(m_canvas->canvasWidget());
    dialog.setWindowTitle(i18n("%1 Options", text())); // TODO add comment (i18nc)
    dialog.addPage(widget, QString());

    if (dialog.exec() == KPageDialog::Accepted && tree->currentItem()) {
        QTreeWidgetItem *item = tree->currentItem();
        if (KoRdfSemanticTreeWidgetItem *ditem = dynamic_cast<KoRdfSemanticTreeWidgetItem*>(item)) {
            kDebug(30015) << "InsertSemanticObjectReferenceAction::activated... item:" << item;
            ditem->insert(m_canvas);
        }
    }
}
