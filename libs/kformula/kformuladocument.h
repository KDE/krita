/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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

#ifndef KFORMULADOCUMENT_H
#define KFORMULADOCUMENT_H

#include <QObject>
#include <QDomDocument>
#include <q3ptrlist.h>
#include <QString>
#include <QStringList>

#include <kaction.h>
#include <kcommand.h>
#include <kconfig.h>
#include <kcommand.h>
//#include "KoCommandHistory.h"
#include "kformuladefs.h"

KFORMULA_NAMESPACE_BEGIN

class Container;
class ContextStyle;
class SymbolAction;
class SymbolTable;
class DocumentWrapper;


/**
 * small utility class representing a sortable (by x,y position) list
 * of formulas you can use sort() and inSort(item)
 **/
class FormulaList: public Q3PtrList<Container>
{
protected:
    virtual int compareItems( Q3PtrCollection::Item a, Q3PtrCollection::Item b );
};


/**
 * A document that can contain a lot of formulas (container).
 *
 * The relationship between the document and its formulas is an
 * open one. The document sure owns the formulas and when it
 * vanishes the formulas will be destroyed, too. But the user
 * will most often work with those formulas directly and not
 * bother to ask the document. It's legal to directly create
 * or destroy a Container object.
 */
class KOFORMULA_EXPORT Document : public QObject {
    Q_OBJECT

    friend class DocumentWrapper;
    friend class Container;

public:

    /**
     * Creates a formula document.
     */
    Document( QObject *parent=0, const char *name=0,
              const QStringList &args=QStringList() );
    ~Document();

    /**
     * Factory method.
     */
    virtual Container* createFormula( int pos=-1, bool registerMe=true );

    /**
     * Registers a new formula to be part of this document. Each formula
     * must be part of exactly one document.
     */
    virtual void registerFormula( Container*, int pos=-1 );

    /**
     * Removes a formula from this document. The formula will stay
     * alive and might be registered again.
     */
    virtual void unregisterFormula( Container* );

    /**
     * Triggers the evaluation of the whole document. This obviously
     * required evaluation support.
     */
    virtual void evaluateFormulas() {}
    virtual void enableEvaluation( bool ) {}

    /**
     * Load a kformula DomDocument with all its formulas.
     * This must only be called on a virgin document.
     */
    bool loadXML( const QDomDocument& doc );

    /**
     * Load a OASIS content.xml DomDocument
     * @since 1.4
     */
    bool loadOasis( const QDomDocument& doc );
    
    /**
     * Load the document settings.
     */
    bool loadDocumentPart( QDomElement node );

    /**
     * Save the document with all its formulae.
     */
    QDomDocument saveXML();

    /**
     * Save the document settings.
     */
    QDomElement saveDocumentPart( QDomDocument& doc );


    /**
     * @returns the documents context style.
     */
    ContextStyle& getContextStyle( bool edit=false );

    /**
     * Change the zoom factor to @p z (e.g. 150 for 150%)
     * and/or change the resolution, given in DPI.
     * Uses the KoTextZoomHandler.
     */
    void setZoomAndResolution( int zoom, int dpiX, int dpiY );

    void newZoomAndResolution( bool updateViews, bool forPrint );

    /**
     * Sets the zoom by hand. This is to be used in <code>paintContent</code>.
     */
    void setZoomAndResolution( int zoom, double zoomX, double zoomY,
                               bool updateViews=false, bool forPrint=false );

    double getXResolution() const;
    double getYResolution() const;

    /**
     * Sets a new formula.
     */
    void activate(Container* formula);

    /**
     * Enables our action according to enabled.
     */
    void setEnabled( bool enabled );

    /**
     * @returns our undo stack so the formulas can use it.
     */
    KCommandHistory* getHistory() const;

    /**
     * @returns the documents symbol table
     */
    const SymbolTable& getSymbolTable() const;

    /**
     * Gets called when the configuration changed.
     * (Maybe we can find a better solution.)
     */
    void updateConfig();

    /**
     * Return a kformula DomDocument.
     */
    static QDomDocument createDomDocument();

public:

    /**
     * @returns an iterator for the collection of formulas.
     */
    Q3PtrListIterator<Container> formulas();

    SymbolType leftBracketChar();
    SymbolType rightBracketChar();

    DocumentWrapper* wrapper() { return m_wrapper; }

protected:

    /**
     * @returns the internal position of this formula or -1 if it
     * doesn't belong to us.
     */
    int formulaPos( Container* formula );

    /**
     * @returns the formula at position pos.
     */
    Container* formulaAt( uint pos );

    /**
     * @returns the number of formulas in this document.
     */
    int formulaCount();

    /**
     * Sorts the list of formulas according to their screen positions.
     */
    void sortFormulaList();

private:

    /**
     * Return the formula with the given number or create a new one
     * if there is no such formula.
     */
    Container* newFormula( uint number );

    /**
     * @returns whether we have a formula that can get requests.
     */
    bool hasFormula();

    /**
     * recalc all formulae.
     */
    void recalc();

    void introduceWrapper( DocumentWrapper* wrapper, bool init );

    /**
     * The Wrapper we belong to.
     */
    DocumentWrapper* m_wrapper;

    /**
     * The active formula.
     */
    Container* m_formula;

    /**
     * The documents context style. This is the place where all
     * the user configurable informations are stored.
     */
    ContextStyle* m_contextStyle;

    /**
     * All formulae that belong to this document.
     */
    FormulaList formulae;
};



/**
 * A Wrapper that constracts the actions and must be given a real
 * document to work with.
 */
class KOFORMULA_EXPORT DocumentWrapper : public QObject {
    Q_OBJECT

public:

    DocumentWrapper( KConfig* config,
                     KActionCollection* collection,
                     KCommandHistory* history = 0 );
    ~DocumentWrapper();

    KConfig* config() { return m_config; }
    KCommandHistory* history() { return m_history; }

    /**
     * @return the document we are using.
     */
    Document* document() const { return m_document; }

    /**
     * Enables our action according to enabled.
     */
    void setEnabled( bool enabled );

    /**
     * Inserts the document we are wrapping. This must be called once
     * before the wrapper can be used.
     */
    void document( Document* document, bool init = true );

    KAction* getAddNegThinSpaceAction()  { return m_addNegThinSpaceAction; }
    KAction* getAddThinSpaceAction()     { return m_addThinSpaceAction; }
    KAction* getAddMediumSpaceAction()   { return m_addMediumSpaceAction; }
    KAction* getAddThickSpaceAction()    { return m_addThickSpaceAction; }
    KAction* getAddQuadSpaceAction()     { return m_addQuadSpaceAction; }
    KAction* getAddBracketAction()       { return m_addBracketAction; }
    KAction* getAddSBracketAction()      { return m_addSBracketAction;}
    KAction* getAddCBracketAction()      { return m_addCBracketAction;}
    KAction* getAddAbsAction()           { return m_addAbsAction;}
    KAction* getAddFractionAction()      { return m_addFractionAction; }
    KAction* getAddRootAction()          { return m_addRootAction; }
    KAction* getAddSumAction()           { return m_addSumAction; }
    KAction* getAddProductAction()       { return m_addProductAction; }
    KAction* getAddIntegralAction()      { return m_addIntegralAction; }
    KAction* getAddMatrixAction()        { return m_addMatrixAction; }
    KAction* getAddOneByTwoMatrixAction(){ return m_addOneByTwoMatrixAction; }
    KAction* getAddUpperLeftAction()     { return m_addUpperLeftAction; }
    KAction* getAddLowerLeftAction()     { return m_addLowerLeftAction; }
    KAction* getAddUpperRightAction()    { return m_addUpperRightAction; }
    KAction* getAddLowerRightAction()    { return m_addLowerRightAction; }
    KAction* getAddGenericUpperAction()  { return m_addGenericUpperAction; }
    KAction* getAddGenericLowerAction()  { return m_addGenericLowerAction; }
    KAction* getAddOverlineAction()      { return m_addOverlineAction; }
    KAction* getAddUnderlineAction()     { return m_addUnderlineAction; }
    KAction* getAddMultilineAction()     { return m_addMultilineAction; }
    KAction* getRemoveEnclosingAction()  { return m_removeEnclosingAction; }
    KAction* getMakeGreekAction()        { return m_makeGreekAction; }
    KAction* getInsertSymbolAction()     { return m_insertSymbolAction; }

    KAction* getAppendColumnAction()     { return m_appendColumnAction; }
    KAction* getInsertColumnAction()     { return m_insertColumnAction; }
    KAction* getRemoveColumnAction()     { return m_removeColumnAction; }
    KAction* getAppendRowAction()        { return m_appendRowAction; }
    KAction* getInsertRowAction()        { return m_insertRowAction; }
    KAction* getRemoveRowAction()        { return m_removeRowAction; }

    void enableMatrixActions(bool);
    KSelectAction* getLeftBracketAction()  { return m_leftBracket; }
    KSelectAction* getRightBracketAction() { return m_rightBracket; }
    SymbolAction* getSymbolNamesAction()  { return m_symbolNamesAction; }
    KToggleAction* getSyntaxHighlightingAction()
        { return m_syntaxHighlightingAction; }
    KToggleAction* getFormatBoldAction()   { return m_formatBoldAction; }
    KToggleAction* getFormatItalicAction() { return m_formatItalicAction; }

    KSelectAction* getFontFamilyAction() { return m_fontFamily; }

    SymbolType leftBracketChar() const  { return m_leftBracketChar; }
    SymbolType rightBracketChar() const { return m_rightBracketChar; }

    void updateConfig();

    KCommandHistory* getHistory() const { return m_history; }

    void undo();
    void redo();

public slots:

    void paste();
    void copy();
    void cut();

    void addNegThinSpace();
    void addThinSpace();
    void addMediumSpace();
    void addThickSpace();
    void addQuadSpace();
    void addDefaultBracket();
    void addBracket( SymbolType left, SymbolType right );
    void addParenthesis();
    void addSquareBracket();
    void addCurlyBracket();
    void addLineBracket();
    void addFraction();
    void addRoot();
    void addIntegral();
    void addProduct();
    void addSum();
    void addMatrix( uint rows=0, uint columns=0 );
    void addOneByTwoMatrix();
    void addNameSequence();
    void addLowerLeftIndex();
    void addUpperLeftIndex();
    void addLowerRightIndex();
    void addUpperRightIndex();
    void addGenericLowerIndex();
    void addGenericUpperIndex();
    void addOverline();
    void addUnderline();
    void addMultiline();
    void removeEnclosing();
    void makeGreek();
    void insertSymbol();
    void insertSymbol( QString name );

    void appendColumn();
    void insertColumn();
    void removeColumn();
    void appendRow();
    void insertRow();
    void removeRow();

    void toggleSyntaxHighlighting();
    void textBold();
    void textItalic();
    void delimiterLeft();
    void delimiterRight();
    void symbolNames();

    void fontFamily();

private:

    void createActions( KActionCollection* collection );
    void initSymbolNamesAction();
    void setCommandStack( KCommandHistory* history );

    bool hasFormula() { return m_document->hasFormula(); }
    Container* formula() { return m_document->m_formula; }

    Document* m_document;

    KAction* m_addNegThinSpaceAction;
    KAction* m_addThinSpaceAction;
    KAction* m_addMediumSpaceAction;
    KAction* m_addThickSpaceAction;
    KAction* m_addQuadSpaceAction;
    KAction* m_addBracketAction;
    KAction* m_addSBracketAction;
    KAction* m_addCBracketAction;
    KAction* m_addAbsAction;
    KAction* m_addFractionAction;
    KAction* m_addRootAction;
    KAction* m_addSumAction;
    KAction* m_addProductAction;
    KAction* m_addIntegralAction;
    KAction* m_addMatrixAction;
    KAction* m_addOneByTwoMatrixAction;
    KAction* m_addUpperLeftAction;
    KAction* m_addLowerLeftAction;
    KAction* m_addUpperRightAction;
    KAction* m_addLowerRightAction;
    KAction* m_addGenericUpperAction;
    KAction* m_addGenericLowerAction;
    KAction* m_addOverlineAction;
    KAction* m_addUnderlineAction;
    KAction* m_addMultilineAction;
    KAction* m_removeEnclosingAction;
    KAction* m_makeGreekAction;
    KAction* m_insertSymbolAction;

    KAction* m_appendColumnAction;
    KAction* m_insertColumnAction;
    KAction* m_removeColumnAction;
    KAction* m_appendRowAction;
    KAction* m_insertRowAction;
    KAction* m_removeRowAction;

    KToggleAction* m_syntaxHighlightingAction;
    KToggleAction* m_formatBoldAction;
    KToggleAction* m_formatItalicAction;

    KSelectAction* m_leftBracket;
    KSelectAction* m_rightBracket;
    SymbolAction* m_symbolNamesAction;

    KSelectAction* m_fontFamily;

    SymbolType m_leftBracketChar;
    SymbolType m_rightBracketChar;
    QString m_selectedName;

    KConfig* m_config;
    KCommandHistory* m_history;

    /**
     * Tells whether we are responsible to remove our history.
     */
    bool m_ownHistory;

    bool m_hasActions;
};


KFORMULA_NAMESPACE_END

#endif // KFORMULADOCUMENT_H
