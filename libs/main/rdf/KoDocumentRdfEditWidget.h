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

#ifndef __koDocumentRdfEditWidget_h__
#define __koDocumentRdfEditWidget_h__

#include "RdfForward.h"
#include "KoDocumentRdfEditWidgetBase.h"
#include <QWidget>

class KoDocumentRdf;
class KoSemanticStylesheet;
class QComboBox;

/**
 * @short A widget to let the user view and edit the Rdf for with the document
 * @author Ben Martin <ben.martin@kogmbh.com>
 * @see KoDocumentRdf
 *
 * This is initially used by the KoDocumentInfoDlg class to add a new page
 * that shows you and editable interface for your Rdf. The class was kept
 * as its own QWidget so that it can be moved to other dialogs and or shown
 * in many places.
 *
 * The widget lets the user edit Rdf that is stored both inline in content.xml
 * as well as from manifest.rdf and other external Rdf files from the OASIS
 * document.
 *
 */
class KoDocumentRdfEditWidget : public KoDocumentRdfEditWidgetBase
{
    Q_OBJECT
public:

    /**
     * The constructor
     * @param parent a pointer to the parent widget
     * @param docRdf a pointer to the KoDocumentRdf to show/edit
     */
    KoDocumentRdfEditWidget(QWidget *parent, KoDocumentRdf *docRdf);

    /** The destructor */
    virtual ~KoDocumentRdfEditWidget();

    /** Add this widget to a user interface where you want Rdf editing */
    QWidget *widget();

    bool shouldDialogCloseBeVetoed();

    /** OK button in dialog, if this returns false then do not close the dialog */
    void apply();

public slots:

    /**
     * Create a new triple in the model and UI in the Triples page.
     */
    void addTriple();

    /**
     * copy the triples which are currently selected in the Triples
     * page. new triples will have a unique identifier appended to
     * their object to avoid attempting to insert the same
     * subj,pred,obj twice.
     */
    void copyTriples();

    /**
     * Delete the selected triples in the Triples page
     */
    void deleteTriples();

    /**
     * Create a new namespace
     */
    void addNamespace();

    /**
     * Delete the selected namespaces
     */
    void deleteNamespace();

    /**
     * Execute the SPARQL query the user has provided.
     *
     * query is taken from m_ui->m_sparqlQuery
     * results are added to m_ui->m_sparqlResultView
     */
    void sparqlExecute();

    /**
     * This methods set the default stylesheet to the
     * user selection for each type of KoRdfSemanticItem.
     */
    void defaultContactsSheetButton();
    void defaultEventsSheetButton();
    void defaultLocationsSheetButton();
    void defaultAllSheetButton();

private slots:

    /**
     * Show a context menu for the semantic treeview
     */
    void showSemanticViewContextMenu(const QPoint &at);

    /**
     * The user edited a semantic item, update the view.
     */
    void semanticObjectUpdated(KoRdfSemanticItem *item);

private:

    hKoSemanticStylesheet stylesheetFromComboBox(QComboBox *w) const;

    class KoDocumentRdfEditWidgetPrivate;
    KoDocumentRdfEditWidgetPrivate *const d;
};
#endif
