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

#ifndef __rdf_RdfSemanticTreeWidgetItem_h__
#define __rdf_RdfSemanticTreeWidgetItem_h__

#include "komain_export.h"
#include "rdf/RdfForward.h"

#include <QObject>
#include <QTreeWidgetItem>
class KAction;
class KoCanvasBase;


/**
 * Code wishing to display a list of RdfSemanticItem objects can create TreeWidgetItems
 * using RdfSemanticItem::createQTreeWidgetItem().
 *
 * These tree widget items can in turn generate a context menu by calling the
 * actions() method. These actions are already setup to work on the underlying
 * RdfSemanticItem objects.
 *
 * @author Ben Martin <ben.martin@kogmbh.com>
 * @see RdfSemanticItem::createQTreeWidgetItem()
 * @see KoDocumentRdf
 */
class KOMAIN_EXPORT RdfSemanticTreeWidgetItem : public QObject, public QTreeWidgetItem
{
    Q_OBJECT

    virtual void testfunc() {}

protected:
    virtual QString uIObjectName() = 0;
    void addApplyStylesheetActions(QWidget *parent, QList<KAction *> &actions, KoCanvasBase *host);
    KAction* createAction(QWidget *parent, KoCanvasBase *host, const QString &text);

public:
    enum {
        Type = QTreeWidgetItem::UserType + 1
    };
    enum {
        // camel case
        COL_NAME = 0,
        COL_SIZE
    };

    RdfSemanticTreeWidgetItem(QTreeWidgetItem *parent, int type);
    virtual ~RdfSemanticTreeWidgetItem();

    /**
     * Get the underlying SemanticItem for this widget
     */
    virtual RdfSemanticItem *semanticItem() = 0;

    virtual QList<KAction *> actions(QWidget *parent, KoCanvasBase *host = 0);
    virtual void insert(KoCanvasBase *host);

public slots:
    virtual void edit();
};
#endif
