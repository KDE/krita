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

#include "KoRdfSemanticTreeWidgetItem.h"

#include "KoDocumentRdf.h"

#include <KoCanvasBase.h>
#include <KoToolProxy.h>
#include <KoDocumentResourceManager.h>
#include <KoTextEditor.h>

#include <KActionMenu>
#include <QVBoxLayout>
#include <KPageDialog>

#include <kdebug.h>
#include <klocale.h>

using namespace Soprano;

class RdfSemanticTreeWidgetApplyStylesheet : public RdfSemanticTreeWidgetAction
{
    hKoRdfSemanticItem si;
    hKoSemanticStylesheet ss;

public:

    RdfSemanticTreeWidgetApplyStylesheet(QWidget *parent, KoCanvasBase *canvas,
                                         const QString &name,
                                         hKoRdfSemanticItem si,
                                         hKoSemanticStylesheet ss = hKoSemanticStylesheet(0));
    virtual ~RdfSemanticTreeWidgetApplyStylesheet();
    virtual void activated();
};

RdfSemanticTreeWidgetApplyStylesheet::RdfSemanticTreeWidgetApplyStylesheet(QWidget *parent,
        KoCanvasBase *canvas, const QString &name, hKoRdfSemanticItem si, hKoSemanticStylesheet ss )
            : RdfSemanticTreeWidgetAction(parent, canvas, name)
            , si(si)
            , ss(ss)
{
}

RdfSemanticTreeWidgetApplyStylesheet::~RdfSemanticTreeWidgetApplyStylesheet()
{
}

void RdfSemanticTreeWidgetApplyStylesheet::activated()
{
    kDebug(30015) << "apply selected stylesheet for semantic item...";
    KoTextEditor *editor = KoTextEditor::getTextEditorFromCanvas(m_canvas);
    const KoDocumentRdf *rdf = si->documentRdf();
    QString xmlid = rdf->findXmlId(editor);
    kDebug(30015) << "semItem:" << si->name() << "xmlid:" << xmlid;
    KoRdfSemanticItemViewSite vs(si, xmlid);
    if (ss) {
        kDebug(30015) << "apply stylesheet, format(), sheet:" << ss->name()
                << " xmlid:" << xmlid;
        vs.applyStylesheet(editor, ss);
    } else {
        vs.disassociateStylesheet();
    }
}


KoRdfSemanticTreeWidgetItem::KoRdfSemanticTreeWidgetItem(QTreeWidgetItem *parent, int Type)
    : QTreeWidgetItem(parent, Type)
{
}

KoRdfSemanticTreeWidgetItem::~KoRdfSemanticTreeWidgetItem()
{
}

KAction *KoRdfSemanticTreeWidgetItem::createAction(QWidget *parent, KoCanvasBase *host, const QString  &text)
{
    return new RdfSemanticTreeWidgetAction(parent, host, text);
}

void KoRdfSemanticTreeWidgetItem::addApplyStylesheetActions(QWidget *parent,
        QList<KAction *> &actions, KoCanvasBase *host)
{
    if (!host) {
        return;
    }
    KoTextEditor *editor = KoTextEditor::getTextEditorFromCanvas(host);
    kDebug(30015) << " semanticItem:" << semanticItem();
    kDebug(30015) << " semanticItem.name:" << semanticItem()->name();
    if (!editor) {
        return;
    }
    QString xmlid = semanticItem()->documentRdf()->findXmlId(editor);
    if (!xmlid.size()) {
        return;
    }
    kDebug(30015) << "xmlid:" << xmlid;
    KActionMenu *topMenu = new KActionMenu(i18n("Apply Stylesheet"), parent);
    actions.append(topMenu);
    KActionMenu *subMenu = new KActionMenu(i18n("System"), topMenu);
    topMenu->addAction(subMenu);
    foreach (hKoSemanticStylesheet ss, semanticItem()->stylesheets()) {
        kDebug(30015) << "format(), sheet:" << ss->name() << " xmlid:" << xmlid;
        KAction* action = new RdfSemanticTreeWidgetApplyStylesheet(parent,
                                                                   host, ss->name(),
                                                                   semanticItem(), ss);
        subMenu->addAction(action);
    }
    subMenu = new KActionMenu(i18n("User"), topMenu);
    topMenu->addAction(subMenu);
    foreach (hKoSemanticStylesheet ss, semanticItem()->userStylesheets()) {
        kDebug(30015) << "format(), sheet:" << ss->name() << " xmlid:" << xmlid;
        KAction *action = new RdfSemanticTreeWidgetApplyStylesheet(parent,
                                                                   host, ss->name(),
                                                                   semanticItem(), ss);
        subMenu->addAction(action);
    }
    // add reapply current sheet option
    topMenu->addSeparator();
    KoRdfSemanticItemViewSite vs(semanticItem(), xmlid);
    if (hKoSemanticStylesheet ss = vs.stylesheet()) {
        KAction *action = new RdfSemanticTreeWidgetApplyStylesheet(parent,
                                                                   host, i18n("Reapply Current"),
                                                                   semanticItem(), ss);
        topMenu->addAction(action);
    }
    KAction *action = new RdfSemanticTreeWidgetApplyStylesheet(parent,
                                                               host, i18n("Disassociate"),
                                                               semanticItem(), hKoSemanticStylesheet(0));
    topMenu->addAction(action);
}

QList<KAction *> KoRdfSemanticTreeWidgetItem::actions(QWidget *parent, KoCanvasBase *host)
{
    Q_UNUSED(parent);
    Q_UNUSED(host);
    return QList<KAction *>();
}

void KoRdfSemanticTreeWidgetItem::insert(KoCanvasBase *host)
{
    Q_UNUSED(host);
    kDebug(30015) << "KoRdfSemanticTreeWidgetItem::insert";
}

void KoRdfSemanticTreeWidgetItem::edit()
{
    QString caption = i18n("Edit %1",uIObjectName());
    QWidget *widget = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(widget);
    widget->setLayout(lay);
    lay->setMargin(0);
    QWidget *w = semanticItem()->createEditor(widget);
    lay->addWidget(w);
    KPageDialog dialog;
    dialog.setCaption(caption);
    dialog.addPage(widget, QString());
    if (dialog.exec() == KPageDialog::Accepted) {
        kDebug(30015) << "KoRdfCalendarEventTreeWidgetItem::edit() accepted...";
        semanticItem()->updateFromEditorData();
    }
}
