/* This file is part of the KDE project
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net> 
                 2009 Jeremias Epperlein <jeeree@web.de>

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
   Boston, MA 02110-1301, USA.
 */

#ifndef KOFORMULATOOL_H
#define KOFORMULATOOL_H

#include <KoToolBase.h>

class KoFormulaShape;
class BasicElement;
class FormulaEditor;
class FormulaCommand;
class QSignalMapper;

/**
 * @short The flake tool for a formula
 * @author Martin Pfeiffer <hubipete@gmx.net>
 */
class KoFormulaTool : public KoToolBase {
    Q_OBJECT
public:
    /// The standard constructor
    explicit KoFormulaTool( KoCanvasBase *canvas );

    /// The standard destructor
    ~KoFormulaTool();

    /// reimplemented
    void paint( QPainter &painter, const KoViewConverter &converter );

    /// reimplemented
    void mousePressEvent( KoPointerEvent *event ) ;

    /// reimplemented
    void mouseDoubleClickEvent( KoPointerEvent *event );

    /// reimplemented
    void mouseMoveEvent( KoPointerEvent *event );

    /// reimplemented
    void mouseReleaseEvent( KoPointerEvent *event );

    void keyPressEvent( QKeyEvent *event );

    void keyReleaseEvent( QKeyEvent *event );

    void remove( bool backSpace );

    /// @return The currently manipulated KoFormulaShape
    KoFormulaShape* shape();

    /// @return The currently active cursor
    FormulaEditor* formulaEditor();

    /// Reset the cursor
    void resetFormulaEditor();

public slots:
    /// Called when this tool instance is activated and fills m_formulaShape
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);

    /// Called when this tool instance is deactivated
    void deactivate();

    /// Insert the element tied to the given @p action
    void insert( const QString& action );

    void changeTable( QAction* action);
    
    void insertSymbol( const QString& symbol);

    /// Reposition the cursor according to the data change
    void updateCursor(FormulaCommand* command, bool undo);

    void saveFormula();

    void loadFormula();

    
protected:
    /// Create default option widget
    QWidget* createOptionWidget();

    virtual void copy() const;

    virtual void deleteSelection();

    virtual bool paste();

    virtual QStringList supportedPasteMimeTypes() const;
    
private:
    /// Repaint the cursor and selection
    void repaintCursor();

    /// Creates all the actions provided by the tool
    void setupActions();

    void addTemplateAction(const QString& caption, const QString& name, const QString& data, const QString& iconName);
    
    /// The FormulaShape the tool is manipulating
    KoFormulaShape* m_formulaShape;

    /// The FormulaEditor the tool uses to move around in the formula
    FormulaEditor* m_formulaEditor;

    QList<FormulaEditor*> m_cursorList;

    QSignalMapper* m_signalMapper;
};

#endif
