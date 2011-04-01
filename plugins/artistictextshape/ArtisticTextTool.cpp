/* This file is part of the KDE project
 * Copyright (C) 2007,2011 Jan Hambrecht <jaham@gmx.net>
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

#include "ArtisticTextTool.h"
#include "AttachTextToPathCommand.h"
#include "DetachTextFromPathCommand.h"
#include "ArtisticTextShapeConfigWidget.h"
#include "MoveStartOffsetStrategy.h"

#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>
#include <KoPathShape.h>
#include <KoShapeController.h>
#include <KoShapeContainer.h>
#include <KoLineBorder.h>
#include <KoInteractionStrategy.h>

#include <KLocale>
#include <KIcon>
#include <KDebug>

#include <QtGui/QAction>
#include <QtGui/QGridLayout>
#include <QtGui/QToolButton>
#include <QtGui/QCheckBox>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QUndoCommand>
#include <QtCore/QPointer>

#include <float.h>

const int BlinkInterval = 500;

class ArtisticTextTool::AddTextRangeCommand : public QUndoCommand
{
    public:
        AddTextRangeCommand( ArtisticTextTool *tool, const QString &text, unsigned int from )
        : m_tool( tool ), m_text( text ), m_from( from )
        {
            m_shape = tool->m_currentShape;
            setText( i18n("Add text range") );
        }
        
        virtual void undo()
        {
            QUndoCommand::undo();
            
            if ( ! m_shape )
                return;
            
            if (m_tool) {
                if ( m_tool->m_currentShape != m_shape ) {
                    m_tool->enableTextCursor( false );
                    m_tool->m_currentShape = m_shape;
                    m_tool->enableTextCursor( true );
                }
                m_tool->setTextCursorInternal( m_from );
                m_tool->m_currentText.remove( m_from, m_text.length() );
            }
            
            m_shape->removeRange( m_from, m_text.length() );
        }
        
        virtual void redo()
        {
            QUndoCommand::redo();
            
            if ( !m_shape )
                return;
            
            if ( m_tool && m_tool->m_currentShape != m_shape ) {
                m_tool->enableTextCursor( false );
                m_tool->m_currentShape = m_shape;
                m_tool->enableTextCursor( true );
            }
            
            m_shape->addRange( m_from, m_text );
            
            if (m_tool) {
                m_tool->setTextCursorInternal( m_from + m_text.length() );
            }
        }
        
    private:
        QPointer<ArtisticTextTool> m_tool;
        ArtisticTextShape *m_shape;
        QString m_text;
        unsigned int m_from;
};

class ArtisticTextTool::RemoveTextRangeCommand : public QUndoCommand
{
    public:
        RemoveTextRangeCommand( ArtisticTextTool *tool, int from, unsigned int count )
        : m_tool( tool ), m_from( from ), m_count( count )
        {
            m_shape = tool->m_currentShape;
            m_cursor = tool->textCursor();
            
            setText( i18n("Remove text range") );
        }
        
        virtual void undo()
        {
            QUndoCommand::undo();
            
            if ( !m_shape )
                return;
            
            if (m_tool) {
                if ( m_tool->m_currentShape != m_shape ) {
                    m_tool->enableTextCursor( false );
                    m_tool->m_currentShape = m_shape;
                    m_tool->enableTextCursor( true );
                }
                m_tool->m_currentShape = m_shape;
                m_tool->m_currentText.insert( m_from, m_text );
            }
            
            m_shape->addRange( m_from, m_text );
            
            if (m_tool) {
                m_tool->setTextCursorInternal( m_cursor );
            }
        }
        
        virtual void redo()
        {
            QUndoCommand::redo();
            
            if ( !m_shape )
                return;
            
            if (m_tool) {
                if ( m_tool->m_currentShape != m_shape ) {
                    m_tool->enableTextCursor( false );
                    m_tool->m_currentShape = m_shape;
                    m_tool->enableTextCursor( true );
                }
                if( m_cursor > m_from )
                    m_tool->setTextCursorInternal( m_from );
            }
            m_text = m_shape->removeRange( m_from, m_count );
        }
        
    private:
        QPointer<ArtisticTextTool> m_tool;
        ArtisticTextShape *m_shape;
        int m_from;
        unsigned int m_count;
        QString m_text;
        int m_cursor;
};


ArtisticTextTool::ArtisticTextTool(KoCanvasBase *canvas)
    : KoToolBase(canvas), m_currentShape(0), m_hoverText(0), m_hoverPath(0), m_hoverHandle(false)
    , m_textCursor( -1 ), m_showCursor( true ), m_currentStrategy(0)
{
    m_detachPath  = new QAction(KIcon("artistictext-detach-path"), i18n("Detach Path"), this);
    m_detachPath->setEnabled( false );
    connect( m_detachPath, SIGNAL(triggered()), this, SLOT(detachPath()) );

    m_convertText  = new QAction(KIcon("pathshape"), i18n("Convert to Path"), this);
    m_convertText->setEnabled( false );
    connect( m_convertText, SIGNAL(triggered()), this, SLOT(convertText()) );

    KoShapeManager *manager = canvas->shapeManager();
    connect( manager, SIGNAL(selectionContentChanged()), this, SLOT(textChanged()));
    connect(manager, SIGNAL(selectionChanged()), this, SLOT(shapeSelectionChanged()));

    setTextMode(true);
}

ArtisticTextTool::~ArtisticTextTool()
{
    delete m_currentStrategy;
}

QTransform ArtisticTextTool::cursorTransform() const
{
    QTransform transform( m_currentShape->absoluteTransformation(0) );
    QPointF pos;
    m_currentShape->getCharPositionAt( m_textCursor, pos );
    transform.translate( pos.x() - 1, pos.y() );
    qreal angle;
    m_currentShape->getCharAngleAt( m_textCursor, angle );
    transform.rotate( 360. - angle );
    if ( m_currentShape->isOnPath() ) {
        QFont f = m_currentShape->font();
        QFontMetrics metrics(f);
        transform.translate( 0, metrics.descent() );
    }

    return transform;
}

void ArtisticTextTool::paint( QPainter &painter, const KoViewConverter &converter)
{
    if (! m_currentShape)
        return;

    if (m_showCursor && m_blinkingCursor.isActive()) {
        painter.save();
        m_currentShape->applyConversion( painter, converter );
        painter.setBrush( Qt::black );
        painter.setWorldTransform( cursorTransform(), true );
        painter.setClipping( false );
        painter.drawPath( m_textCursorShape );
        painter.restore();
    }
    m_showCursor = !m_showCursor;

    if (m_currentShape->isOnPath()) {
        painter.save();
        m_currentShape->applyConversion( painter, converter );
        painter.setPen(Qt::blue);
        painter.setBrush(m_hoverHandle ? Qt::red : Qt::white);
        painter.drawPath(offsetHandleShape());
        painter.restore();
    }
}

void ArtisticTextTool::repaintDecorations()
{
    canvas()->updateCanvas(offsetHandleShape().boundingRect());
}

void ArtisticTextTool::mousePressEvent( KoPointerEvent *event )
{
    if (m_hoverHandle) {
        enableTextCursor(false);
        m_currentStrategy = new MoveStartOffsetStrategy(this, m_currentShape);
    }
    if (m_hoverText) {
        KoSelection *selection = canvas()->shapeManager()->selection();
        if(m_hoverText != m_currentShape) {
            // if we hit another text shape, select that shape
            selection->deselectAll();
            enableTextCursor( false );
            m_currentShape = m_hoverText;
            m_currentText = m_currentShape->text();
            emit shapeSelected(m_currentShape, canvas());
            enableTextCursor( true );
            selection->select( m_currentShape );
        }
        // change the text cursor position
        QPointF pos = event->point;
        pos -= m_currentShape->absolutePosition( KoFlake::TopLeftCorner );
        const int len = m_currentShape->text().length();
        int hit = len;
        qreal mindist = DBL_MAX;
        for ( int i = 0; i < len;++i ) {
            QPointF center;
            m_currentShape->getCharPositionAt( i, center );
            center = pos - center;
            if ( (fabs(center.x()) + fabs(center.y())) < mindist ) {
                hit = i;
                mindist = fabs(center.x()) + fabs(center.y());
            }
        }
        setTextCursorInternal( hit );
    }
    event->ignore();
}

void ArtisticTextTool::mouseMoveEvent( KoPointerEvent *event )
{
    m_hoverPath = 0;
    m_hoverText = 0;

    if (m_currentStrategy) {
        m_currentStrategy->handleMouseMove(event->point, event->modifiers());
        return;
    }

    const bool textOnPath = m_currentShape && m_currentShape->isOnPath();
    if (textOnPath) {
        QPainterPath handle = offsetHandleShape();
        QPointF handleCenter = handle.boundingRect().center();
        if (handleGrabRect(event->point).contains(handleCenter)) {
            // mouse is on offset handle
            if (!m_hoverHandle)
                canvas()->updateCanvas(handle.boundingRect());
            m_hoverHandle = true;
        } else {
            if (m_hoverHandle)
                canvas()->updateCanvas(handle.boundingRect());
            m_hoverHandle = false;
        }
    }
    if (!m_hoverHandle) {
        // find text or path shape at cursor position
        QList<KoShape*> shapes = canvas()->shapeManager()->shapesAt( handleGrabRect(event->point) );
        if (shapes.contains(m_currentShape)) {
            m_hoverText = m_currentShape;
        } else {
            foreach( KoShape * shape, shapes ) {
                ArtisticTextShape * text = dynamic_cast<ArtisticTextShape*>( shape );
                if (text && !m_hoverText) {
                    m_hoverText = text;
                }
                KoPathShape * path = dynamic_cast<KoPathShape*>( shape );
                if (path && !m_hoverPath) {
                    m_hoverPath = path;
                }
                if(m_hoverPath && m_hoverText)
                    break;
            }
        }
    }

    const bool hoverOnBaseline = textOnPath && m_currentShape->baselineShape() == m_hoverPath;
    // update cursor and status text
    if ( m_hoverText ) {
        useCursor( QCursor( Qt::IBeamCursor ) );
        if (m_hoverText == m_currentShape)
            emit statusTextChanged(i18n("Click to change cursor position."));
        else
            emit statusTextChanged(i18n("Click to select text shape."));
    } else if( m_hoverPath && m_currentShape && !hoverOnBaseline) {
        useCursor( QCursor( Qt::PointingHandCursor ) );
        emit statusTextChanged(i18n("Double click to put text on path."));
    } else  if (m_hoverHandle) {
        useCursor( QCursor( Qt::ArrowCursor ) );
        emit statusTextChanged( i18n("Drag handle to change start offset.") );
    } else {
        useCursor( QCursor( Qt::ArrowCursor ) );
        if (m_currentShape)
            emit statusTextChanged( i18n("Press return to finish editing.") );
        else
            emit statusTextChanged("");
    }
}

void ArtisticTextTool::mouseReleaseEvent( KoPointerEvent *event )
{
    if (m_currentStrategy) {
        m_currentStrategy->finishInteraction(event->modifiers());
        QUndoCommand *cmd = m_currentStrategy->createCommand();
        if (cmd)
            canvas()->addCommand(cmd);
        delete m_currentStrategy;
        m_currentStrategy = 0;
        enableTextCursor(true);
    }
    updateActions();
}

void ArtisticTextTool::mouseDoubleClickEvent(KoPointerEvent */*event*/)
{
    if (m_hoverPath && m_currentShape) {
        if (!m_currentShape->isOnPath() || m_currentShape->baselineShape() != m_hoverPath) {
            m_blinkingCursor.stop();
            m_showCursor = false;
            updateTextCursorArea();
            canvas()->addCommand( new AttachTextToPathCommand( m_currentShape, m_hoverPath ) );
            m_blinkingCursor.start( BlinkInterval );
            updateActions();
            m_hoverPath = 0;
            return;
        }
    }
}

void ArtisticTextTool::keyPressEvent(QKeyEvent *event)
{
    event->accept();
    if ( m_currentShape && textCursor() > -1 ) {
        switch(event->key())
        {
        case Qt::Key_Delete:
            if( textCursor() >= 0 && textCursor() < m_currentShape->text().length())
                removeFromTextCursor( textCursor(), 1 );
            break;    
        case Qt::Key_Backspace:
            removeFromTextCursor( textCursor()-1, 1 );
            break;
        case Qt::Key_Right:
            setTextCursor( textCursor() + 1 );
            break;
        case Qt::Key_Left:
            setTextCursor( textCursor() - 1 );
            break;
        case Qt::Key_Home:
            setTextCursor( 0 );
            break;
        case Qt::Key_End:
            setTextCursor( m_currentShape->text().length() );
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            emit done();
            break;
        default:
            addToTextCursor( event->text() );
        }
    } else {
        event->ignore();
    }
}

void ArtisticTextTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    Q_UNUSED(toolActivation);
    foreach (KoShape *shape, shapes) {
        m_currentShape = dynamic_cast<ArtisticTextShape*>( shape );
        if(m_currentShape)
            break;
    }
    if( m_currentShape == 0 )  {
        // none found
        emit done();
        return;
    } else {
        m_currentText = m_currentShape->text();
        emit shapeSelected(m_currentShape, canvas());
    }

    m_hoverText = 0;
    m_hoverPath = 0;

    createTextCursorShape();
    enableTextCursor( true );
    updateActions();
    emit statusTextChanged( i18n("Press return to finish editing.") );
}

void ArtisticTextTool::blinkCursor()
{
    updateTextCursorArea();
}

void ArtisticTextTool::deactivate()
{
    if ( m_currentShape ) {
        if( m_currentShape->text().isEmpty() ) {
            canvas()->addCommand( canvas()->shapeController()->removeShape( m_currentShape ) );
        }
        enableTextCursor( false );
        m_currentShape = 0;
    }
    m_hoverPath = 0;
    m_hoverText = 0;
}

void ArtisticTextTool::updateActions()
{
    if( m_currentShape ) {
        m_detachPath->setEnabled( m_currentShape->isOnPath() );
        m_convertText->setEnabled( true );
    } else {
        m_detachPath->setEnabled( false );
        m_convertText->setEnabled( false );
    }
}

void ArtisticTextTool::detachPath()
{
    if( m_currentShape && m_currentShape->isOnPath() )
    {
        canvas()->addCommand( new DetachTextFromPathCommand( m_currentShape ) );
        updateActions();
    }
}

void ArtisticTextTool::convertText()
{
    if( ! m_currentShape )
        return;

    KoPathShape * path = KoPathShape::createShapeFromPainterPath( m_currentShape->outline() );
    path->setParent( m_currentShape->parent() );
    path->setZIndex( m_currentShape->zIndex() );
    path->setBorder( m_currentShape->border() );
    path->setBackground( m_currentShape->background() );
    path->setTransformation( m_currentShape->transformation() );
    path->setShapeId( KoPathShapeId );

    QUndoCommand * cmd = canvas()->shapeController()->addShapeDirect( path );
    cmd->setText( i18n("Convert to Path") );
    canvas()->shapeController()->removeShape( m_currentShape, cmd );
    canvas()->addCommand( cmd );

    emit done();
}

QMap<QString, QWidget *> ArtisticTextTool::createOptionWidgets()
{
    QMap<QString, QWidget *> widgets;
    
    QWidget * pathWidget = new QWidget();
    pathWidget->setObjectName("ArtisticTextPathWidget");
    
    QGridLayout * layout = new QGridLayout(pathWidget);
    
    QToolButton * detachButton = new QToolButton(pathWidget);
    detachButton->setDefaultAction( m_detachPath );
    layout->addWidget( detachButton, 0, 0 );
    
    QToolButton * convertButton = new QToolButton(pathWidget);
    convertButton->setDefaultAction( m_convertText );
    layout->addWidget( convertButton, 0, 2 );
    
    layout->setSpacing(0);
    layout->setMargin(6);
    layout->setRowStretch(3, 1);
    layout->setColumnStretch(1, 1);
    
    widgets.insert(i18n("Text On Path"), pathWidget);
    
    ArtisticTextShapeConfigWidget * configWidget = new ArtisticTextShapeConfigWidget();
    configWidget->setObjectName("ArtisticTextConfigWidget");
    if (m_currentShape) {
        configWidget->initializeFromShape(m_currentShape, canvas());
    }
    connect(this, SIGNAL(shapeSelected(ArtisticTextShape *, KoCanvasBase *)), 
            configWidget, SLOT(initializeFromShape(ArtisticTextShape *, KoCanvasBase *)));
    connect(canvas()->shapeManager(), SIGNAL(selectionContentChanged()),
            configWidget, SLOT(updateWidget()));
            
    widgets.insert(i18n("Text Properties"), configWidget);
    
    return widgets;
}

void ArtisticTextTool::enableTextCursor( bool enable )
{
    if ( enable ) {
        if( m_currentShape )
            setTextCursorInternal( m_currentShape->text().length() );
        connect( &m_blinkingCursor, SIGNAL(timeout()), this, SLOT(blinkCursor()) );
        m_blinkingCursor.start( BlinkInterval );
    } else {
        m_blinkingCursor.stop();
        disconnect( &m_blinkingCursor, SIGNAL(timeout()), this, SLOT(blinkCursor()) );
        setTextCursorInternal( -1 );
        m_showCursor = false;
    }
}

void ArtisticTextTool::setTextCursor( int textCursor )
{
    if ( m_textCursor == textCursor || textCursor < 0 
    || ! m_currentShape || textCursor > m_currentShape->text().length() )
        return;

    setTextCursorInternal( textCursor );
}

void ArtisticTextTool::updateTextCursorArea() const
{
    if( ! m_currentShape || m_textCursor < 0 )
        return;

    QRectF bbox = cursorTransform().mapRect( m_textCursorShape.boundingRect() );
    canvas()->updateCanvas( bbox );
}

void ArtisticTextTool::setTextCursorInternal( int textCursor )
{
    updateTextCursorArea();
    m_textCursor = textCursor;
    updateTextCursorArea();
}

void ArtisticTextTool::createTextCursorShape()
{
    if ( m_textCursor < 0 || ! m_currentShape ) 
        return;
    m_textCursorShape = QPainterPath();
    QRectF extents;
    m_currentShape->getCharExtentsAt( m_textCursor, extents );
    m_textCursorShape.addRect( 0, 0, 1, -extents.height() );
    m_textCursorShape.closeSubpath();
}

void ArtisticTextTool::removeFromTextCursor( int from, unsigned int count )
{
    if ( from >= 0 ) {
        m_currentText.remove( from, count );
        QUndoCommand *cmd = new RemoveTextRangeCommand( this, from, count );
        canvas()->addCommand( cmd );
    }
}

void ArtisticTextTool::addToTextCursor( const QString &str )
{
    if ( !str.isEmpty() && m_textCursor > -1 ) {
        QString printable;
        for ( int i = 0;i < str.length();i++ ) {
            if ( str[i].isPrint() )
                printable.append( str[i] );
        }
        unsigned int len = printable.length();
        if ( len ) {
            m_currentText.insert( m_textCursor, printable );
            QUndoCommand *cmd = new AddTextRangeCommand( this, printable, m_textCursor );
            canvas()->addCommand( cmd );
        }
    }
}

void ArtisticTextTool::textChanged()
{
    if ( !m_currentShape || m_currentShape->text() == m_currentText )
        return;

    kDebug() << "shape text =" << m_currentShape->text();
    kDebug() << "current text =" << m_currentText;
    
    setTextCursorInternal( m_currentShape->text().length() );
}

void ArtisticTextTool::shapeSelectionChanged()
{
    KoSelection *selection = canvas()->shapeManager()->selection();
    if (selection->isSelected(m_currentShape))
        return;

    foreach (KoShape *shape, selection->selectedShapes()) {
        ArtisticTextShape *text = dynamic_cast<ArtisticTextShape*>(shape);
        if(text) {
            enableTextCursor( false );
            m_currentShape = text;
            emit shapeSelected(m_currentShape, canvas());
            enableTextCursor( true );
            break;
        }
    }
}

QPainterPath ArtisticTextTool::offsetHandleShape()
{
    QPainterPath handle;
    if (!m_currentShape || !m_currentShape->isOnPath())
        return handle;

    const QPainterPath baseline = m_currentShape->baseline();
    const qreal offset = m_currentShape->startOffset();
    QPointF offsetPoint = baseline.pointAtPercent(offset);
    QSizeF paintSize = handlePaintRect(QPointF()).size();

    handle.moveTo(0, 0);
    handle.lineTo(0.5*paintSize.width(), paintSize.height());
    handle.lineTo(-0.5*paintSize.width(), paintSize.height());
    handle.closeSubpath();

    QTransform transform;
    transform.translate( offsetPoint.x(), offsetPoint.y() );
    transform.rotate(360. - baseline.angleAtPercent(offset));

    return transform.map(handle);
}

#include <ArtisticTextTool.moc>
