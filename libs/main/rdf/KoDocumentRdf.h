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

#ifndef KO_DOCUMENT_Rdf_H
#define KO_DOCUMENT_Rdf_H

#include "komain_export.h"
#include "KoDocumentRdfBase.h"

#include "KoXmlReaderForward.h"
#include <KoDataCenterBase.h>
#include <kconfig.h>

#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <Soprano/Soprano>
#include <QTextBlockUserData>
#include <QTreeWidgetItem>
#include <kaction.h>
#include <kdatetime.h>

class QDomDocument;
class KoStore;
class KoXmlWriter;
class KoDocument;
class KoCanvasBase;
class KoTextEditor;

#include "RdfForward.h"
#include "KoSemanticStylesheet.h"
#include "KoRdfSemanticItem.h"
#include "KoRdfSemanticItemViewSite.h"
#include "RdfSemanticTreeWidgetAction.h"
#include "InsertSemanticObjectActionBase.h"
#include "InsertSemanticObjectCreateAction.h"
#include "InsertSemanticObjectReferenceAction.h"
#include "KoRdfSemanticTree.h"

class KoRdfCalendarEvent;
class KoRdfLocation;
class KoDocumentRdfPrivate;
class KoRdfFoaF;


/**
 * @short The central access point for the Rdf metadata of an ODF document.
 *
 * @author Ben Martin <ben.martin@kogmbh.com>
 * @see KoDocument
 * @see KoDocumentInfo
 *
 * The KoDocumentRdf object is possibly associated with a KoDocument.
 * There does not need to be a KoDocumentRdf for each KoDocument, but
 * if one exists it is a one-to-one relationship. The KoDocumentRdf
 * is also associated with the KoDocumentResourceManager of a canvas.
 *
 * Once again, the canvas to KoDocumentRdf is either a 1-1 or 1 to
 * zero relationship.
 *
 * ACCESS TO Rdf:
 *
 * You can get at the Rdf information in two main ways: either using
 * Soprano/SPARQL or through KoRdfSemanticItem objects.
 *
 * Subclasses of KoRdfSemanticItem exist for locations (foaf,vcard),
 * events (ical), and locations (two Rdf geolocation formats). To get
 * a list of these objects use the foaf(), calendarEvents(), and
 * locations() methods of this class. Each of these methods optionally
 * takes a Soprano::Model and returns a list of SemanticItems of a
 * particular subclass. If you do not pass an Soprano::Model to the
 * methods the default model() of the KoDocumentRdf is used. By
 * allowing you to pass a model explicitly, you can find contacts that
 * exist in a subset of the full Rdf graph for a document. This is
 * useful if you want to find the contacts in the users current
 * "selection" in the document.
 *
 * For example, to find the foaf entries related to the current KoTextEditor
 *
 * Soprano::Model* model = rdf->findStatements( editor );
 * KoRdfFoaFList foaflist = rdf->foaf( model );
 *
 * Using the Soprano::Model directly is covered in a latter section of
 * this comment.
 *
 * STORAGE OF Rdf:
 *
 * Broadly there are two ways Rdf metadata is stored in an ODF
 * document.
 *
 * 1) inline in the content.xml file in an Rdfa style, though not
 *    using the full Rdfa spec
 *
 * 2) externally in manifest.rdf or other Rdf/XML files linked to by
 * manifest.rdf
 *
 * The Rdf that is stored using these methods is collected and made
 * available by the KoDocumentRdf class. The inline Rdf using option
 * (1) is stored along with the Calligra C++ objects that are created
 * during document loading. This class also knows how to find the
 * scattered Rdf that option (1) loads. Leaving the Rdf from option
 * (1) scattered in the document allows it to be preserved in the
 * normal course of document editing such as copy and paste, undo and
 * redo operations.
 *
 * The scattered Rdf from option (1) is stored using the
 * KoTextInlineRdf class. You can convert a KoTextInlineRdf to
 * a Soprano::Statement with the toStatement() method of this class.
 *
 * LOW LEVEL Rdf ACCESS:
 *
 * The model() method will give you a Soprano::model with all the Rdf
 * for the document, be them from option (1) or (2) above.
 *
 * The findStatements() methods will give you a Soprano::model
 * containing the statements relevant to a cursor or xml:id in the
 * document. The xml:id is the same identifier that is used in the
 * content.xml file that was loaded.
 *
 * Note that the findStatements() returns a submodel containing only
 * the statements relevant to the xml:id or cursor position you
 * selected. This will be a subset of all the Rdf for the document.
 * The various expand() methods can be used to add more Rdf statements
 * from the document to the submodel returned by findStatements(). For
 * example, the expandStatementsReferencingSubject() method will
 * expand the soprano::model to add Rdf statements which refer to any
 * subject in the Rdf submodel you pass in.
 */
class KOMAIN_EXPORT KoDocumentRdf : public KoDocumentRdfBase
{
    Q_OBJECT
public:

    /**
     * The constructor
     * @param parent a pointer to the parent object
     */
    KoDocumentRdf(QObject *parent = 0);

    /** The destructor */
    ~KoDocumentRdf();

    /**
     * Load from an OASIS document
     * @param metaDoc the QDomDocument with the metaInformation
     * @return true if success
     */
    bool loadOasis(KoStore *store);

    /**
     * Save to an OASIS document
     * @param store a pointer to a KoStore to save in
     * @return true if success
     */
    bool saveOasis(KoStore *store, KoXmlWriter *manifestWriter);

    /**
     * Used by KoRdfSemanticItem when creating new semantic items so that the
     * KoDocumentRdf class can find them.
     */
    void rememberNewInlineRdfObject(KoTextInlineRdf *inlineRdf);

    /**
     * Find all the KoTextInlineRdf objects that exist in the
     * document and update the statements in the Soprano::(model) to
     * reflect the current state of the inline Rdf.
     */
    void updateInlineRdfStatements(const QTextDocument *qdoc);

    /**
     * During a save(), various Rdf objects in the document will
     * create new xmlid values which are used in the saved document.
     * As part of the save state a QMap from the old xmlid to the new
     * xmlid is built up. This method is then used to update the Rdf
     * triples to use the new xmlid values before that Rdf itself is
     * saved. This way the Rdf -> xmlid references will remain valid
     * in the saved ODF file.
     *
     * Since the Rdf is updated to use the new xmlid values, this method
     * also updates the C++ objects to use the new xmlid values so that
     * the instance of the document in memory is correctly linked.
     *
     * This way, Calligra is free to change the xml:id during save() and
     * the Rdf is still linked correctly.
     */
    void updateXmlIdReferences(const QMap<QString, QString> &m);

    /**
     * Get the namespace to URI prefix mapping object.
     */
    KoRdfPrefixMapping* prefixMapping() const;

    /**
     * Get the Soprano::Model that contains all the Rdf
     * You do not own the model, do not delete it.
     */
    virtual QSharedPointer<Soprano::Model> model() const;

    /**
     * Convert an inlineRdf object into a Soprano::Statement
     */
    Soprano::Statement toStatement(KoTextInlineRdf *inlineRdf) const;

    /**
     * Look for the semitem with the given xmlid and return the
     * start and end position for that semitem. If there is no semitem
     * with the \p xmlId then 0,0 is returned.
     */
    QPair<int, int> findExtent(const QString &xmlId) const;

    /**
     * Look for the semitem that is at or surrounding the cursor given. Note that if there
     * are nested semitems, the extend for the most nested semitem is returned.
     * for example, in the below senario the return value will be QPair< start-b, end-b >.
     *
     * <start-a> ... <start-b> ... cursor ... <end-b> ... <end-a>
     */
    QPair<int, int> findExtent(KoTextEditor *handler) const;

    /**
     * find the xmlid of the semitem that is at or surrounding the cursor given. As with
     * findExtent() this will be only the most nested semitem.
     * @see findExtent()
     */
    QString findXmlId(KoTextEditor *cursor) const;


    /**
     * Find all of the statements which are
     * in context for a given cursor position.
     *
     * Rdf is also added to the returned model from external manifest.rdf
     * and other files which attach to the xml:id
     *
     * The depth parameter allows triple expansion, ie,
     *        depth=1   adds only triples which reference ?s ?p xml:id
     *        depth=2   adds references to statements in depth=1,
     *                  ie, ?s1 ?p2 ?s2 and ?s2 ?p xml:id
     * Because of the complexity, depth should be <=2 or you should use
     * a custom query.
     *
     * FIXME: The logical thing here would be to chain up.
     *   given a cursor in a table:cell, the Rdf for the containing
     *   text:p and text:meta elements should be returned too.
     *
     * Note that the returned model is owned by the caller, you must delete it.
     */
    QSharedPointer<Soprano::Model> findStatements(const QString &xmlid, int depth = 1);
    QSharedPointer<Soprano::Model> findStatements(KoTextEditor *handler, int depth = 1);

    /**
     * Add all the Rdf that is associated with the given xml:id
     */
    void addStatements(QSharedPointer<Soprano::Model> model, const QString &xmlid);

    /**
     * Find an inline Rdf object from the xml:id which
     * it has in the content.xml file
     */
    KoTextInlineRdf* findInlineRdfByID(const QString &xmlid) const;

    /**
     * Obtain a list of Contact/FOAF semantic objects, if any, for the Rdf
     * in the default model() or the one you optionally pass in.
     */
    QList<hKoRdfFoaF> foaf(QSharedPointer<Soprano::Model> m = QSharedPointer<Soprano::Model>(0));


    /**
     * Obtain a list of calendar/vevent semantic objects, if any, for the Rdf
     * in the default model() or the one you optionally pass in.
     */
    QList<hKoRdfCalendarEvent> calendarEvents(QSharedPointer<Soprano::Model> m = QSharedPointer<Soprano::Model>(0));

    /**
     * Obtain a list of location semantic objects, if any, for the Rdf
     * in the default model() or the one you optionally pass in.
     */
    QList<hKoRdfLocation> locations(QSharedPointer<Soprano::Model> m = QSharedPointer<Soprano::Model>(0));

    /**
     * For Rdf stored in manifest.rdf or another rdf file referenced
     * by the manifest, this prefix is used as the start of the graph
     * context. The filename.rdf is appended so that the Rdf can be
     * put back into the right file again during save.
     */
    QString rdfPathContextPrefix() const;

    /**
     * This is used for triples that do not specify their xhtml:about
     * ie, the subject URI.
     */
    QString rdfInternalMetadataWithoutSubjectURI() const;

    /**
     * Soprano::Node that can be used as the model context for
     * statements which should be stored in the manifest.rdf file.
     */
    Soprano::Node manifestRdfNode() const;

    /**
     * Soprano::Node that can be used as the model context for
     * statements which were stored in the context.xml file.
     */
    Soprano::Node inlineRdfContext() const;

    /**
     * If model contains ?s ?p ?o
     * look for and add
     * ?s2 ?p2 ?s
     */
    void expandStatementsReferencingSubject(QSharedPointer<Soprano::Model> model) const;

    /**
     * If model contains ?s ?p ?o
     * look for and add
     * ?o ?p2 ?o2
     */
    void expandStatementsSubjectPointsTo(QSharedPointer<Soprano::Model> model) const;

    /**
     * Add n ?p ?o from m_model to model
     */
    void expandStatementsSubjectPointsTo(QSharedPointer<Soprano::Model> model, const Soprano::Node &n) const;

    /**
     * Rdf allows for linked lists to be serialized as a graph. This method will
     * ensure that all data from m_model for any lists that are started in 'model'
     * is copied into 'model'.
     *
     * Lists have the format
     * prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
     * ?id    rdf:first  ?value
     * ?id    rdf:rest   ?next1
     * ?next1 rdf:first  ?value2
     * ?next1 rdf:rest   ?nextN
     * ?nextN rdf:first  ?valueN
     * ?nextN rdf:rest   rdf:nil
     */
    void expandStatementsToIncludeRdfLists(QSharedPointer<Soprano::Model> model) const;

    /**
     * If model contains ?s ?p ?o
     * look for and add
     * ?s ?p3 ?o3
     */
    void expandStatementsToIncludeOtherPredicates(QSharedPointer<Soprano::Model> model) const;

    /**
     * One round of all expandStatements methods
     */
    void expandStatements(QSharedPointer<Soprano::Model> model) const;

    /**
     * XXXX? What does this do?
     */
    KAction* createInsertSemanticObjectReferenceAction(KoCanvasBase *host);

    /**
     * XXXX? What does this do?
     */
    QList<KAction*> createInsertSemanticObjectNewActions(KoCanvasBase *host);

    /**
     * Collect together the semantic item, stylehseet, xmlid of the
     * site to apply it at and the extent in the document (start,end)
     * of the semantic item. Used when applying stylesheets in bulk so
     * that all the sites can be collected and the QMap<int,reflowItem> map
     * will sort them in the order of start to end document position.
     *
     * @see insertReflow()
     * @see applyReflow()
     */
    struct reflowItem
    {
        hKoRdfSemanticItem m_si;
        hKoSemanticStylesheet m_ss;
        QString m_xmlid;
        QPair<int, int> m_extent;

        reflowItem(hKoRdfSemanticItem si, const QString &xmlid, hKoSemanticStylesheet ss, const QPair<int, int> &extent);
    };

    /**
     * Because applying a stylesheet to a semantic item could change
     * the length of the text showing the item in the document, it is
     * best to apply these stylesheets to semitems starting from the
     * end of the document. This is because any change in the length
     * of a semitem has no effect on all the other semitems that are
     * before it in the document. After we have applied all the
     * changes, the kwdoc will update the positions of all the
     * kotextmeta etc objects and they will once again be correct.
     * Doing things explicitly backwards is a huge efficiency gain
     * because no layout is needed on the document during our updates
     * because each update does not invalidate the positions of any
     * objects before the update in the document text.
     *
     * Call insertReflow() for all the items you want to apply a
     * stylesheet on and then applyReflow() with the built up
     * collection 'col' argument to will actually apply the
     * stylesheets starting from the semitem lowest in the document
     * and working backwards.
     *
     * @see applyReflow()
     */
    void insertReflow(QMap<int, reflowItem> &col, hKoRdfSemanticItem obj, hKoSemanticStylesheet ss);
    void insertReflow(QMap<int, reflowItem> &col, hKoRdfSemanticItem obj, const QString &sheetType, const QString &stylesheetName);
    void insertReflow(QMap<int, reflowItem> &col, hKoRdfSemanticItem obj);
    /**
     * @short Apply the stylesheets built up with insertReflow().
     *
     * @see insertReflow()
     */
    void applyReflow(const QMap<int, reflowItem> &col);

    /**
     * For debugging, output the model and a header string for identification
     */
    void dumpModel(const QString &msg, QSharedPointer<Soprano::Model> m = QSharedPointer<Soprano::Model>(0)) const;

signals:
    /**
     * Emitted when a new semanticItem is created so that dockers can
     * update themselves accordingly. It is expected that when
     * semanticObjectViewSiteUpdated is emitted the view will take care
     * of reflowing the semanitc item using it's stylesheet.
     */
    void semanticObjectAdded(hKoRdfSemanticItem item) const;
    void semanticObjectUpdated(hKoRdfSemanticItem item) const;
    void semanticObjectViewSiteUpdated(hKoRdfSemanticItem item, const QString &xmlid) const;

public:
    void emitSemanticObjectAdded(hKoRdfSemanticItem item) const;
    void emitSemanticObjectUpdated(hKoRdfSemanticItem item);
    void emitSemanticObjectViewSiteUpdated(hKoRdfSemanticItem item, const QString &xmlid);
    void emitSemanticObjectAddedConst(hKoRdfSemanticItem const item) const;

    /**
     * You should use the KoRdfSemanticItem::userStylesheets() method instead of this one.
     * This is mainly an internal method to allow user stylesheets to be managed per document.
     */
    QList<hKoSemanticStylesheet> userStyleSheetList(const QString& className) const;
    void setUserStyleSheetList(const QString& className,const QList<hKoSemanticStylesheet>& l);


private:

    /**
     * @see expandStatementsToIncludeRdfLists()
     */
    void expandStatementsToIncludeRdfListsRecurse(QSharedPointer<Soprano::Model> model,
            QList<Soprano::Statement> &addList,
            const Soprano::Node &n) const;


    /**
     * Soprano can give undesirable behaviour when loading two files
     * into the same model. When parsing the second Rdf file, the sane
     * genid1 numbers can be reused, leading to semantic errors on
     * bnodes. This method updates all the bnodes in 'm' to be new
     * ones created using m_model->createBlankNode(). bnode identity
     * is preserved for the model m. ie. genid2 and another genid2 in
     * m will be replaced with the same m_model->createBlankNode()
     * value. After calling this method, you can add all the
     * statements to 'm' and be assured that no bnodes in 'm' are
     * going to accidentially be the same as a bnode in m_model.
     */
    void freshenBNodes(QSharedPointer<Soprano::Model> m);

    /**
     * Used by loadOasis() to load Rdf from a particular external
     * Rdf/XML file.
     */
    bool loadRdf(KoStore *store, const Soprano::Parser *parser, const QString &fileName);

    /**
     * Used by saveOasis() to save Rdf to a the Rdf file nominated
     * with context. Note that this method can not be used to save to
     * content.xml, those Rdf statements must be saved as the
     * content.xml file is generated.
     */
    bool saveRdf(KoStore *store, KoXmlWriter *manifestWriter, const Soprano::Node &context) const;


    /**
     * Because there are at least two different ways of associating digital longitude
     * and latitude with triples in Rdf, the locations() method farms off discovering
     * these values to this method using specific SPARQL query text.
     *
     * @see locations()
     */
    void addLocations(QSharedPointer<Soprano::Model> m, QList<hKoRdfLocation> &ret,
                      bool isGeo84, const QString &sparql);


    /**
     * idrefList queries soprano after loading and creates a list of all rdfid's that
     * where found in the manifest.rdf document. This list is used to make sure we do not
     * create more inline rdf objects than necessary
     * @return a list of xml-id's
     */
    QStringList idrefList() const;

private:

    /**
     * Test whether a model is present that supports:
     * - context / graphs
     *  - querying on graphs
     *  - storage in memory.
     */
    bool backendIsSane();

    /// reimplemented
    virtual bool completeLoading(KoStore *store);

    /// reimplemented
    virtual bool completeSaving(KoStore *store, KoXmlWriter *manifestWriter, KoShapeSavingContext *context);

    KoDocumentRdfPrivate * const d;
};

#endif
