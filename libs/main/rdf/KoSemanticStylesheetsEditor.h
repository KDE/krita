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

#ifndef __rdf_KoSemanticStylesheetsEditor_h__
#define __rdf_KoSemanticStylesheetsEditor_h__

#include "komain_export.h"

#include "RdfForward.h"
#include <KDialog>
#include <QSharedPointer>

class QTreeWidgetItem;
class QTableWidgetItem;

/**
 * @short A dialog to allow the user to see the system stylesheet definitions
 *        and create and edit user stylesheets.
 *
 * @author Ben Martin <ben.martin@kogmbh.com>
 * @see KoDocument
 */
class KOMAIN_EXPORT KoSemanticStylesheetsEditor : public KDialog
{
    Q_OBJECT
public:
    KoSemanticStylesheetsEditor(QWidget *parent, KoDocumentRdf *rdf);
    ~KoSemanticStylesheetsEditor();

protected slots:
    void slotOk();
    void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void newStylesheet();
    void deleteStylesheet();
    void onVariableActivated(QTableWidgetItem *item);
    void definitionChanged();

private:

    void setupStylesheetsItems(const QString &semanticClass,
                               hKoRdfSemanticItem si,
                               const QList<hKoSemanticStylesheet> &ssl,
                               const QMap<QString, QTreeWidgetItem*> &m,
                               bool editable = false);
    void maskButtonsDependingOnCurrentItem(QTreeWidgetItem *current);

    class Private;
    QSharedPointer<Private> d;
};

#endif
