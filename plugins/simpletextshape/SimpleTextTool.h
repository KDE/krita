/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SIMPLETEXTTOOL_H
#define SIMPLETEXTTOOL_H

#include "SimpleTextShape.h"

#include <KoTool.h>

#include <QTimer>
#include <QUndoCommand>

class QAction;

/// This is the tool for the simple text shape.
class SimpleTextTool : public KoTool 
{
    Q_OBJECT
public:
    explicit SimpleTextTool(KoCanvasBase *canvas);
    ~SimpleTextTool();

    /// reimplemented
    virtual void paint( QPainter &painter, const KoViewConverter &converter );

    /// reimplemented
    virtual void mousePressEvent( KoPointerEvent *event ) ;
    /// reimplemented
    virtual void mouseMoveEvent( KoPointerEvent *event );
    /// reimplemented
    virtual void mouseReleaseEvent( KoPointerEvent *event );
    /// reimplemented
    virtual void activate (bool temporary=false);
    /// reimplemented
    virtual void deactivate();
    /// reimplemented
    virtual QWidget *createOptionWidget();
    /// reimplemented
    virtual void keyPressEvent(QKeyEvent *event);
    /// reimplemented
    virtual void deleteSelection();

protected:
    void enableTextCursor( bool enable );
    int textCursor() const { return m_textCursor; }
    void setTextCursor( int textCursor );
    void removeFromTextCursor( int from, unsigned int nr );
    void addToTextCursor( const QString &str );

private slots:
    void attachPath();
    void detachPath();
    void convertText();
    void blinkCursor();
    void textChanged();

private:
class AddTextRange : public QUndoCommand
{
public:
    AddTextRange( SimpleTextTool *tool, const QString &text, unsigned int from )
        : m_tool( tool ), m_text( text ), m_from( from )
    {
        m_shape = tool->m_currentShape;
        setText( "Add text range" );
    }
    virtual void undo()
    {
        if ( m_shape ) {
	    if ( m_tool->m_currentShape != m_shape ) {
	        m_tool->enableTextCursor( false );
		m_tool->m_currentShape = m_shape;
	        m_tool->enableTextCursor( true );
	    }
            m_tool->setTextCursorInternal( m_from );
	    m_tool->m_currentText.remove( m_from, m_text.length() );
            m_shape->removeRange( m_from, m_text.length() );
	}
    }
    virtual void redo()
    {
        if ( m_shape ) {
	    if ( m_tool->m_currentShape != m_shape ) {
	        m_tool->enableTextCursor( false );
		m_tool->m_currentShape = m_shape;
	        m_tool->enableTextCursor( true );
	    }
            m_shape->addRange( m_from, m_text );
            m_tool->setTextCursorInternal( m_from + m_text.length() );
	}
    }
private:
    SimpleTextTool *m_tool;
    SimpleTextShape *m_shape;
    QString m_text;
    unsigned int m_from;
};

class RemoveTextRange : public QUndoCommand
{
public:
    RemoveTextRange( SimpleTextTool *tool, int from, unsigned int nr )
        : m_tool( tool ), m_from( from ), m_nr( nr )
    {
        m_shape = tool->m_currentShape;
        setText( "Remove text range" );
    }
    virtual void undo()
    {
        if ( m_shape ) {
	    if ( m_tool->m_currentShape != m_shape ) {
	        m_tool->enableTextCursor( false );
		m_tool->m_currentShape = m_shape;
	        m_tool->enableTextCursor( true );
	    }
	    m_tool->m_currentShape = m_shape;
	    m_tool->m_currentText.insert( m_from, m_text );
            m_shape->addRange( m_from, m_text );
            m_tool->setTextCursorInternal( m_from + m_nr );
	}
    }
    virtual void redo()
    {
        if ( m_shape ) {
	    if ( m_tool->m_currentShape != m_shape ) {
	        m_tool->enableTextCursor( false );
		m_tool->m_currentShape = m_shape;
	        m_tool->enableTextCursor( true );
	    }
            m_tool->setTextCursorInternal( m_from );
            m_text = m_shape->removeRange( m_from, m_nr );
        }
    }

private:
    SimpleTextTool *m_tool;
    SimpleTextShape *m_shape;
    int m_from;
    unsigned int m_nr;
    QString m_text;
};

private:
    void updateActions();
    void setTextCursorInternal( int textCursor );
    void createTextCursorShape();
    void updateTextCursorArea() const;

    SimpleTextShape * m_currentShape;
    KoPathShape * m_path;
    KoPathShape * m_tmpPath;
    QPainterPath m_textCursorShape;

    QAction * m_attachPath;
    QAction * m_detachPath;
    QAction * m_convertText;

    int m_textCursor;
    QTimer m_blinkingCursor;
    bool m_showCursor;
    QString m_currentText;
};

#endif // SIMPLETEXTTOOL_H
