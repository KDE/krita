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

#ifndef KFORMULACONTAINER_H
#define KFORMULACONTAINER_H

#include <QClipboard>
#include <QImage>
#include <q3ptrlist.h>
#include <QObject>
#include <q3ptrstack.h>
#include <QString>
//Added by qt3to4:
#include <QTextStream>
#include <QKeyEvent>
#include <QDomElement>
#include <kcommand.h>
//#include <KoCommandHistory.h>
#include "kformuladefs.h"

class QColorGroup;
class QKeyEvent;
class QPainter;

class KCommand;
class KPrinter;

KFORMULA_NAMESPACE_BEGIN

class BasicElement;
class Document;
class FormulaCursor;
class FormulaElement;
class IndexElement;
class PlainCommand;
class SymbolTable;


/**
 * The interface the elements expect from its document.
 *
 * Please don't mistake this class for Document.
 * This one represents one formula, the other one provides
 * the context in which the formulae exist.
 */
class FormulaDocument {
    // not allowed
    FormulaDocument( const FormulaDocument& ) {}
    FormulaDocument& operator=( const FormulaDocument& ) { return *this; }
public:

    FormulaDocument() {}
    virtual ~FormulaDocument() {}

    virtual void elementRemoval(BasicElement* /*child*/) {}
    virtual void changed() {}
    virtual void cursorHasMoved( FormulaCursor* ) {}
    virtual void moveOutLeft( FormulaCursor* ) {}
    virtual void moveOutRight( FormulaCursor* ) {}
    virtual void moveOutAbove( FormulaCursor* ) {}
    virtual void moveOutBelow( FormulaCursor* ) {}
    virtual void tell( const QString& /*msg*/ ) {}
    virtual void insertFormula( FormulaCursor* ) {}
    virtual void removeFormula( FormulaCursor* ) {}
    virtual void baseSizeChanged( int, bool ) {}
    virtual const SymbolTable& getSymbolTable() const = 0;
};


/**
 * The document. Actually only one part of the whole.
 * Provides everything to edit the formula.
 */
class KOFORMULA_EXPORT Container : public QObject, public FormulaDocument {
    friend class MimeSource;
    Q_OBJECT

    // no copying
    Container( const Container& );
    Container& operator= ( const Container& );

public:

    enum ViewActions { EXIT_LEFT, EXIT_RIGHT,
                       EXIT_ABOVE, EXIT_BELOW,
                       INSERT_FORMULA, REMOVE_FORMULA };

    /**
     * Constructs a new formula and register it with the document.
     *
     * @param doc the document we belong to.
     * @param pos the formulas position inside its document.
     * @param registerMe whether the formula is to be registered
     * with the document.
     */
    Container( Document* doc, int pos, bool registerMe=true );
    ~Container();

    /**
     * Needs to be called before anything else can be done with a
     * newly created formula! This is required to allow polymorphic
     * formulas. (The virtual method problem.)
     */
    void initialize();

    /**
     * Returns a new cursor. It points to the beginning of the
     * formula. The cursor gets no messages if the formula changes
     * so use this with care!
     */
    FormulaCursor* createCursor();

    /**
     * Gets called just before the child is removed from
     * the element tree.
     */
    void elementRemoval(BasicElement* child);

    /**
     * Gets called when ever something changes and we need to
     * recalc.
     */
    void changed();

    /**
     * Gets called when a request has the side effect of moving the
     * cursor. In the end any operation that moves the cursor should
     * call this.
     */
    void cursorHasMoved( FormulaCursor* );

    /**
     * Inside the formula occurred an event that must be handled
     * outside.
     */
    void moveOutLeft( FormulaCursor* );
    void moveOutRight( FormulaCursor* );
    void moveOutAbove( FormulaCursor* );
    void moveOutBelow( FormulaCursor* );
    void tell( const QString& msg );
    void removeFormula( FormulaCursor* );

    /**
     * Register and unregister this formula with its document.
     */
    void registerFormula( int pos=-1 );
    void unregisterFormula();

    /**
     * The base size changed. If not owned it uses the default one now.
     */
    void baseSizeChanged( int size, bool owned );

    /**
     * Draws the whole thing.
     */
    void draw( QPainter& painter, const QRect& r,
               const QPalette& palette, bool edit=false );

    /**
     * Draws the whole thing.
     */
    void draw( QPainter& painter, const QRect& r, bool edit=false );

    /**
     * Saves the data into the document.
     */
    void save( QDomElement &root );

    /**
     * Save formula as MathML.
     */
    void saveMathML( QTextStream& stream, bool oasisFormat = false );

    /**
     * Load function.
     * Load the formula from the specified file containing MathML .
     */
    bool loadMathML( const QDomDocument &doc, bool oasisFormat = false );

    /**
     * Loads a formula from the document.
     */
    bool load( const QDomElement &fe );

    /**
     * @returns Tex string for the formula
     */
    QString texString();

    QString formulaString();

    /**
     * Prints the formula.
     */
    void print(KPrinter& printer);

    /**
     * @returns an image that looks like out formula.
     */
    QImage drawImage( int width, int height );

    /**
     * @returns the cursor to be used for editing.
     */
    FormulaCursor* activeCursor();
    const FormulaCursor* activeCursor() const;

    /**
     * Sets the cursor that is to be used for any editing.
     *
     * The active cursor might 0. In this case you must not
     * request any change from the formula.
     */
    void setActiveCursor(FormulaCursor* cursor);

    /**
     * @returns the formula's size.
     */
    QRect boundingRect() const;

    /**
     * @returns the formula's size including its active cursor.
     */
    QRect coveredRect();

    double width() const;
    double height() const;

    /**
     * @returns the distance between the upper bound and the formulas
     * base line.
     */
    double baseline() const;

    /**
     * Moves the formula to a new location. This location will be the
     * upper left corner of the rectangle that is drawn by the painter.
     */
    void moveTo( int x, int y );

    /**
     * KWord uses a transformed painter to draw formulas, so every
     * formula has the internal position (0,0). But we might need to
     * sort our formulas according to their position inside the
     * document. (This is only needed for math formulas.)
     */
    virtual double getDocumentX() const { return -1; }
    virtual double getDocumentY() const { return -1; }
    virtual void setDocumentPosition( double /*x*/, double /*y*/ ) {}

    /**
     * Start the documents evaluation at this formula. This must be the
     * formula that changed. The formulas above it won't be affected
     * by this change.
     *
     * This has no meaning in not evaluating formulas.
     */
    virtual void startEvaluation() {}

    /**
     * Recalcs the formula and emits the .*Changed signals if needed.
     */
    void testDirty();

    /**
     * Recalc the formula.
     */
    virtual void recalc();

    /**
     * @returns true if there is no element.
     */
    bool isEmpty();

    /**
     * @returns the document this formula belongs to.
     */
    virtual Document* document() const;

    virtual const SymbolTable& getSymbolTable() const;

    int fontSize() const;

    /**
     * Sets the base font size of this formula.
     */
    void setFontSize( int pointSize, bool forPrint = false );

    void setFontSizeDirect( int pointSize );

    /**
        If the cursor is in a matrix, the matrix actions will be enabled, otherwise disabled.
    */
    void updateMatrixActions();

signals:

    /**
     * The cursor has been moved but the formula hasn't been
     * changed. The view that owns the cursor needs to know this.
     */
    void cursorMoved(FormulaCursor* cursor);

    /**
     * The cursor wants to leave this formula.
     */
    void leaveFormula( Container* formula, FormulaCursor* cursor, int cmd );

    /**
     * The formula has changed and needs to be redrawn.
     */
    void formulaChanged( int width, int height );
    void formulaChanged( double width, double height );

    /**
     * A message that might be a useful hint. Meant for the statusbar.
     */
    void statusMsg( const QString& msg );

    /**
     * A message that describes an error. Meant for a message box. (?)
     */
    void errorMsg( const QString& );

    /**
     * The element is going to leave the formula with and all its children.
     */
    void elementWillVanish(BasicElement* element);

    /**
     * Tells the cursors that we have just loaded a new formula.
     */
    void formulaLoaded(FormulaElement*);

    /**
     * We've got a new base size.
     */
    void baseSizeChanged( int );

public:

    /**
     * General input.
     */
    void input( QKeyEvent* event );

    void performRequest( Request* request );

    // There are a lot of thing we can do with the formula.

    /**
     * Insert data from the clipboard.
     */
    void paste();

    /**
     * Insert data from the document.
     */
    void paste( const QDomDocument& document, QString desc );

    /**
     * Copy the current selection to the clipboard.
     */
    void copy();

    /**
     * Copy and remove.
     */
    void cut();

protected:

    KCommandHistory* getHistory() const;

    /**
     * @returns the root of our element tree. That's the formula's structure.
     */
    FormulaElement* rootElement() const;

    /**
     * Factory method.
     */
    virtual FormulaElement* createMainSequence();

    void emitErrorMsg( const QString& );

private:

    /**
     * Execute the command if it makes sense.
     */
    void execute(KCommand *command);

    /**
     * Emits a signal if the cursor had moved.
     */
    void checkCursor();

    /**
     * @returns true if there is a cursor that is allowed to edit the formula.
     */
    bool hasValidCursor() const;

    struct Container_Impl;
    Container_Impl* impl;

    // debug
    friend class TestFormulaCursor;
    friend class TestFormulaElement;
    friend class TestIndexElement;
    friend class TestCommands;
};

KFORMULA_NAMESPACE_END

#endif // KFORMULACONTAINER_H
