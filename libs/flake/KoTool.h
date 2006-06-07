/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#ifndef KOTOOL_H
#define KOTOOL_H

#include <QString>
#include <QObject>
#include <QCursor>

#include <koffice_export.h>

class KoCanvasBase;
class KoPointerEvent;
class KoViewConverter;

class KActionCollection;
class KRadioAction;

class QKeyEvent;
class QWidget;
class QPainter;

/**
 * Abstract base class for all tools that manipulate flake objects.
 * There exists an instance of every tool for every pointer device.
 *
 * XXX: Note that we also need tools to manipulate the contents of
 *      the objects (e.g., brushes, gradients, text tools, whatever.
 *      It may be preferable to remove the tool system from flake and
 *      make it more general.
 */
class FLAKE_EXPORT KoTool : public QObject
{
    Q_OBJECT
public:

    /**
     * Create a new tool. A tool has a user-visible name,
     * a string that uniquely identifies the tool and a
     * type string that is used to classify the tool, for instance
     * in groups in the toolbox.
     *
     * @param name the user-visible name of this tool
     * @param id the system-id of this tool
     * @param type a string identifying the type of this tool
     * @param canvas the canvas interface this tool will work for.
     */
    KoTool(KoCanvasBase *canvas );
    virtual ~KoTool() {}

public:

    /**
     * Return the option widget for this tool. Create it if it
     * does not exist yet.
     *
     * Note: by default an empty widget is created.
     */
    virtual QWidget * optionWidget(QWidget * parent);

    /// request a repaint of the decorations to be made.
    virtual void repaintDecorations() {};

public slots:
    /**
     * This method is called when this tool instance is activated.
     *
     * @param temporary if true, this tool is only temporarily actived
     *                  and should emit done when it is done.
     *
     * XXX: Make virtual
     */
    virtual void activate(bool temporary = false);

    /**
     * This method is called whenever this tool is no longer the
     * active tool
     */
    virtual void deactivate();


public:

    /**
     * Automatically scroll the canvas with the tool
     */
    virtual bool wantsAutoScroll();

    /**
     * Return the cursor associated with this tool. The application is
     * repsonsible for calling this method and setting the cursor on
     * the right widget.
     *
     * XXX: Make this abstract.
     * XXX: Make it possible to use fancy big cursors
     */
    virtual QCursor cursor( const QPointF &position );

    /**
     * Paint temporary stuff, like rubber bands, grids etc.

     * XXX: Perhaps do this in a more coordinated fashion, instead
     *      of just passing a painter.
     */
    virtual void paint( QPainter &painter, KoViewConverter &converter ) = 0;

public: // Events

    /**
     * Called when (one of) the mouse buttons is pressed.
     * Implementors should call event->ignore() if they do not actually use the event.
     * @param event state and reason of this mouse press
     */
    virtual void mousePressEvent( KoPointerEvent *event ) = 0;

    /**
     * Called when (one of) the mouse buttons is double clicked.
     * Implementors should call event->ignore() if they do not actually use the event.
     * Default implementation ignores this event.
     * @param event state and reason of this mouse press
     */
    virtual void mouseDoubleClickEvent( KoPointerEvent *event );

    /**
     * Called when the mouse moved over the canvas.
     * Implementors should call event->ignore() if they do not actually use the event.
     * @param event state and reason of this mouse move
     */
    virtual void mouseMoveEvent( KoPointerEvent *event ) = 0;

    /**
     * Called when (one of) the mouse buttons is released.
     * Implementors should call event->ignore() if they do not actually use the event.
     * @param event state and reason of this mouse release
     */
    virtual void mouseReleaseEvent( KoPointerEvent *event ) = 0;

    /**
     * Called when a key is pressed.
     * Implementors should call event->ignore() if they do not actually use the event.
     * Default implementation ignores this event.
     * @param event state and reason of this key press
     */
    virtual void keyPressEvent(QKeyEvent *event);

    /**
     * Called when a key is released
     * Implementors should call event->ignore() if they do not actually use the event.
     * Default implementation ignores this event.
     * @param event state and reason of this key release
     */
    virtual void keyReleaseEvent(QKeyEvent *event);

signals:

    /**
     * Emitted when this tool wants itself to be replaced by the
     * specified tool
     *
     *@param id the identification of the desired tool
     */
    void sigActivateTool( const QString &id );

    /**
     * Emitted when this tool wants the specified tool to temporarily
     * replace itself. For instance, a paint tool could desire to be
     * temporarily replaced by a pan tool which could be temporarily
     * replaced by a colourpicker.
     */
    void sigActivateTemporary(const QString &);

    /**
     * Emitted when the tool has been temporarily activated and wants
     * to notify the world that it's done.
     */
    void sigDone();

protected:
    QWidget * m_optionWidget;
    KoCanvasBase *m_canvas; ///< the canvas interface this tool will work for.

private:
    KoTool();
    KoTool(const KoTool&);
    KoTool& operator=(const KoTool&);
};

#endif /* KOTOOL_H */
