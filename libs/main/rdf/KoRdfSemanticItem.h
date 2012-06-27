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

#ifndef __rdf_KoRdfSemanticItem_h__
#define __rdf_KoRdfSemanticItem_h__

#include "komain_export.h"

#include <QObject>
#include <QSharedData>
#include <QExplicitlySharedDataPointer>
#include <QMimeData>
#include <kdatetime.h>
#include <Soprano/Soprano>
#include "RdfForward.h"
#include "KoSemanticStylesheet.h"

class KoCanvasBase;
class QTreeWidgetItem;


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
class KOMAIN_EXPORT KoRdfSemanticItem : public QObject, public QSharedData
{
    Q_OBJECT

public:

    KoRdfSemanticItem(QObject *parent);
    KoRdfSemanticItem(const KoDocumentRdf *rdf, QObject *parent);
    KoRdfSemanticItem(const KoDocumentRdf *m_rdf, Soprano::QueryResultIterator &it, QObject *parent);
    virtual ~KoRdfSemanticItem();


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

    /**
     * The importFromData() method can use this method to finish an
     * import. Text is also inserted into the document to show the
     * user the new semantic object. The semanticObjectAdded signal is
     * emitted so that dockers have a chance to update themselves to
     * reflect the newly added SemanticItem in the document.
     *
     * This method uses createEditor() followed by
     * updateFromEditorData() to actually put the Rdf triples into the
     * store. So a subclass does not have to explicitly handle the
     * import if it can present a GUI to edit itself. The GUI is not
     * shown to the user.
     *
     */
    virtual void importFromDataComplete(const QByteArray &ba, const KoDocumentRdf *rdf = 0, KoCanvasBase *host = 0);

    friend class KoSemanticStylesheetsEditor;
    friend class KoSemanticStylesheet;
    virtual void setupStylesheetReplacementMapping(QMap<QString, QString> &m);

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
     * perform actions at the various places the semanitc item appears
     * in the document.
     */
    QStringList xmlIdList() const;

    /**
     * Create a QWidget that can edit the SemanticItem. Note that the
     * widget will show the data and allow editing of it for the
     * SemanticItem, but to make changes permenant, the
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
     * Create a QTreeWidgetItem to display this SemanticItem. This
     * method should be used if you want to present a QTree of
     * SemanticItems because the returned widgetItem can also create a
     * menu and perform other actions for the SemanticItem.
     */
    virtual KoRdfSemanticTreeWidgetItem *createQTreeWidgetItem(QTreeWidgetItem *parent = 0);

    /**
     * Insert the SemanticItem into the document at the current cursor
     * position. The appearance of each semantic item is likely to be
     * different depending on the user's current formating
     * preferences. For example, a contact might show one or more of
     * the person's names and their phone number.
     *
     * This method inserts markers and other book keeping and uses
     * the default stylesheet to insert a representation of the
     * SemanticItem.
     */
    virtual void insert(KoCanvasBase *host);

    /**
     * Export the SemanticItem to MimeData. This mehtod is used by
     * Drag and Drop to allow the item to move to another application
     * or possibly onto the clipboard. Subclasses might like to use a
     * QTemporaryFile and call their exportToFile() method to export
     * themselves to MIME data. For maximum usability a plain text
     * represenation should also be set with md->setText() so items
     * can be dragged to text editors and consoles.
     */
    virtual void exportToMime(QMimeData *md) const;

    /**
     * Export to a file in whatever format is the most useful for the
     * semantic item. Prompt for a filename if none is given.
     */
    virtual void exportToFile(const QString &fileName = QString()) const = 0;

    /**
     * Import the data in ba to the semnatic item. This is used for
     * D&D Drop events to create a new semnatic item. Subclasses
     * should set their internal state based on the data in 'ba' and
     * then call importFromDataComplete() at the end of the method to
     * update the Rdf and insert the semantic item into the document.
     *
     * This method calls also insert() which links the semanticItem with the
     * KoDocumentRdf object m_rdf.
     */
    virtual void importFromData(const QByteArray &ba, KoDocumentRdf *rdf = 0, KoCanvasBase *host = 0) = 0;

    /**
     * A simple description of the semantic item that can be shown to the user
     */
    virtual QString name() const = 0;

    /**
     * Name of the subclass as would be contained in classNames()
     */
    virtual QString className() const = 0;

    /**
     * Gets a list of SemanticItem subclasses that can be created.
     * Any of the strings in the return value can be created using
     * createSemanticItem().
     *
     * @see createSemanticItem()
     */
    static QStringList classNames();

    /**
     * Create a SemanticItem subclass using its name from
     * classNames(). Useful for menus and other places that want to
     * allow the user to create new SemanticItem Objects.
     */
    static QExplicitlySharedDataPointer<KoRdfSemanticItem> createSemanticItem(QObject *parent, const KoDocumentRdf *rdf, const QString &semanticClass);

    /**
     * Get the system semantic stylesheets that are supported for this
     * particular semantic item subclass.
     */
    virtual QList<hKoSemanticStylesheet> stylesheets() const = 0;
    /**
     * Get the user created/editable semantic stylesheets that are
     * supported for this particular semantic item subclass.
     */
    QList<hKoSemanticStylesheet> userStylesheets() const;

    /**
     * Unambiguiously find a stylesheet by its UUID. The sheet can
     * be either user or system as long as it has the uuid you want.
     */
    hKoSemanticStylesheet findStylesheetByUuid(const QString &uuid) const;

    /**
     * Find a user/system stylesheet by name.
     * sheetType is one of TYPE_SYSTEM/TYPE_USER.
     * n is the name of the stylesheet you want.
     */
    hKoSemanticStylesheet findStylesheetByName(const QString &sheetType, const QString &n) const;
    /**
     * Find a user/system stylesheet by name.
     * ssl is either stylesheets() or userStylesheets()
     * n is the name of the stylesheet you want.
     */
    hKoSemanticStylesheet findStylesheetByName(const QList<hKoSemanticStylesheet> &ssl, const QString &n) const;

    /**
     * Get the default stylesheet for this subclass of Semantic Item.
     *
     * If you want to know which stylesheet is in use by a particular
     * reference to a semantic item, use KoRdfSemanticItemViewSite::stylesheet()
     *
     * @see KoRdfSemanticItemViewSite
     * @see KoRdfSemanticItemViewSite::stylesheet()
     */
    hKoSemanticStylesheet defaultStylesheet() const;
    /**
     * Set the default stylesheet for this subclass of Semantic Item.
     *
     * If you want to set the stylesheet for a particular reference to a
     * semantic item, use KoRdfSemanticItemViewSite::applyStylesheet().
     * @see KoRdfSemanticItemViewSite::applyStylesheet()
     */
    void defaultStylesheet(hKoSemanticStylesheet ss);

    /**
     * Create a new user stylesheet
     */
    hKoSemanticStylesheet createUserStylesheet(const QString &name, const QString &templateString = QString());

    /**
     * Destroy a user stylesheet
     */
    void destroyUserStylesheet(hKoSemanticStylesheet ss);

    /**
     * Load the user stylesheets from the given Rdf model. They are
     * assumed to be in the format saved by saveUserStylesheets()
     *
     * @see saveUserStylesheets()
     */
    void loadUserStylesheets(QSharedPointer<Soprano::Model> model);
    /**
     * Save the user stylesheets to the Rdf model given.
     *
     * Stylesheets are saved as an Rdf list of stylesheet bnodes. If
     * there is already a list in the model it is removed first and
     * then the current collection of user stylesheets is added into
     * the Rdf model. Note that as the collection of user stylesheets
     * exists on a per subclass basis, this method saves the list to
     * the Rdf model using a list head that depends on which subclass
     * you are saving the user stylesheets of. As such, you should
     * save and load the user stylesheets for each class in
     * classNames().
     *
     * @see loadUserStylesheets()
     * @see classNames()
     */
    void saveUserStylesheets(QSharedPointer<Soprano::Model> model, const Soprano::Node &context) const;


protected slots:
    /**
     * In case the stylesheets move to using a QMap<String,sheet> or
     * we want to know when a stylesheet has been renamed.
     */
    void onUserStylesheetRenamed(hKoSemanticStylesheet ss, const QString &oldName, const QString &newName);

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

#endif
