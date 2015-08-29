/* This file is part of the KDE project
 *   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this library; see the file COPYING.LIB.  If not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __rdf_KoRdfBasicSemanticItem_h__
#define __rdf_KoRdfBasicSemanticItem_h__

#include "kordf_export.h"

#include "RdfForward.h"
// KDE
#include <KDateTime>
// Soprano
#include <Soprano/Soprano>
// Qt
#include <QSharedData>

//TODO: rework this documentation
/**
 * @short Base class for C++ objects which represent Rdf at a higher level.
 * @author Ben Martin <ben.martin@kogmbh.com>
 *
 * Base class for Semantic Items (semitems). A semantic item is
 * created from one or more Rdf triples and brings that related
 * information together into a C++ object. For example, for contact
 * information, many Rdf triples conforming to the FOAF specification
 * might be present.
 *
 * Code can call createQTreeWidgetItem() to create an item that can be
 * displayed to the user without needing to know about triples or Rdf.
 *
 * @see KoRdfSemanticTreeWidgetItem
 * @see KoDocumentRdf
 *
 */
class KORDF_EXPORT KoRdfBasicSemanticItem : public QObject, public QSharedData
{
    Q_OBJECT

public:
    explicit KoRdfBasicSemanticItem(QObject *parent);
    KoRdfBasicSemanticItem(QObject *parent, const KoDocumentRdf *rdf);
    KoRdfBasicSemanticItem(QObject *parent, const KoDocumentRdf *rdf, Soprano::QueryResultIterator &it);
    virtual ~KoRdfBasicSemanticItem();


protected:
    /**
     * Create a bnode with a uuid
     */
    Soprano::Node createNewUUIDNode() const;

public:
    /**
     * For an item like a contact, event, location, if there is a
     * subject, common#idref xml:id triple that can be used to link
     * into the document content, return that subject node here for
     * the common base class methods to use.
     *
     * For example, in the FOAF vocabulary the ?person node from the
     * SPARQL query fragment below will be the linkingSubject()
     * ?person rdf:type foaf:Person
     */
    virtual Soprano::Node linkingSubject() const;

protected:
    /**
     * Return the graph context that contains this SematicItem's Rdf
     * statements. Used by the updateTriple()s to remove and add
     * updated information. The default is the manifest.rdf file.
     */
    virtual Soprano::Node context() const;

    /**
     * When a subclass wants to update a triple in the Rdf store
     * to reflect a change, for example, the phone number of a
     * contact, it should call here to set toModify = newValue.
     *
     * This is done both in the C++ objects and the Rdf model.
     * The Rdf will be changed from
     * linkingSubject() predString toModify
     * to
     * linkingSubject() predString newValue
     *
     * Note that rounding errors and other serialization issues that
     * crop up are handled by these methods, so you should try very
     * hard not to directly update the Soprano::Model outside these
     * methods.
     */
    void updateTriple(QString &toModify, const QString &newValue, const QString &predString);
    void updateTriple(KDateTime &toModify, const KDateTime &newValue, const QString &predString);
    void updateTriple(double &toModify, double newValue, const QString &predString, const Soprano::Node &explicitLinkingSubject);

    /**
     * Ensure the Rdf Type of the linkingSubject is what you want
     * After this method, the Rdf will have the following:
     * linkingSubject() rdf:type type
     */
    void setRdfType(const QString &type);

public:
    /**
     * The document Rdf object that this semantic item is associated with.
     */
    const KoDocumentRdf *documentRdf() const;

    /**
     * A Semantic Item can appear multiple times in a document. For
     * example, the person Frodo can be referenced 20 times in a short
     * book review. This method gives all the xmlid values where the
     * semantic item is referenced in the document.
     *
     * The list of xmlid values can in turn be used by
     * KoDocumentRdf::findExtent() and findStatements() to inspect or
     * perform actions at the various places the semantic item appears
     * in the document.
     */
    QStringList xmlIdList() const;

    /**
     * Create a QWidget that can edit the SemanticItem. Note that the
     * widget will show the data and allow editing of it for the
     * SemanticItem, but to make changes permanent, the
     * updateFromEditorData() method must be used. A typical senario
     * is to add the widget from createEditor to a dialog and when the
     * user affirms the dialog call updateFromEditorData() to update
     * the document.
     *
     * @see updateFromEditorData()
     */
    virtual QWidget *createEditor(QWidget *parent) = 0;

    /**
     * Update the SemanticItem from the edited dialog that was created using
     * createEditor.
     *
     * @see createEditor()
     */
    virtual void updateFromEditorData() = 0;

    /**
     * Name of the subclass as would be contained in classNames()
     */
    virtual QString className() const = 0;

private:
    /**
     * The updateTriple() methods all call remove() then add() to
     * perform their work. These lower level functions accept
     * Soprano::LiteralValues to remove/add. Note that corner cases
     * like "double" values are explicitly handled by these methods.
     * For example, at times a double will undergo some rounding
     * during serialization, so you can not just call
     * Soprano::Model.removeStatement() because you have to take
     * rounding errors into account for the value you are intending to
     * remove.
     */
    void updateTriple_remove(const Soprano::LiteralValue &toModify,
                             const QString &predString,
                             const Soprano::Node& explicitLinkingSubject);

    /**
     * After updateTriple() calls remove() it can set toModify to the
     * new value and call this method to add the new value to the Rdf
     * store.
     */
    void updateTriple_add(const Soprano::LiteralValue &toModify,
                          const QString &predString,
                          const Soprano::Node &explicitLinkingSubject);

protected:
    const KoDocumentRdf *m_rdf;    //< For access to the Rdf model during CRUD operations
    Soprano::Node m_context; //< This determines the Rdf/XML file the Rdf is stored in (see context())
};

#endif //__rdf_KoRdfBasicSemanticItem_h__
