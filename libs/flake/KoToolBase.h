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
#include <QStringList>
#include <QtCore/QRectF>

#include "flake_export.h"

class KoShape;
class KoCanvasBase;
class KoPointerEvent;
class KoViewConverter;
class KoToolSelection;
class KoToolBasePrivate;

class KAction;
class QAction;
class QKeyEvent;
class QWidget;
class QPainter;
class QInputMethodEvent;

/**
 * Abstract base class for all tools. Tools can create or manipulate
 * flake shapes, canvas state or any other thing that a user may wish
 * to do to his document or his view on a document with a pointing
 * device.
 *
 * There exists an instance of every tool for every pointer device.
 * These instances are managed by the toolmanager..
 */
class FLAKE_EXPORT KoToolBase : public QObject
{
    Q_OBJECT
public:
    /// Option for activate()
    enum ToolActivation {
        TemporaryActivation, ///< The tool is activated temporarily and works 'in-place' of another one.
        DefaultActivation   ///< The tool is activated normally and emitting 'done' goes to the defaultTool
    };
    /// Action information
    enum ReadWrite {
        ReadOnlyAction, ///< Used for actions that do not alter the content
        ReadWriteAction ///< Used to signify actions that can change document content
    };


    /**
     * Constructor, normally only called by the factory (see KoToolFactoryBase)
     * @param canvas the canvas interface this tool will work for.
     */
    explicit KoToolBase(KoCanvasBase *canvas);
    virtual ~KoToolBase();

    /**
     * request a repaint of the decorations to be made. This triggers
     * an update call on the canvas, but does not paint directly.
     */
    virtual void repaintDecorations();

    /**
     * Return if dragging (moving with the mouse down) to the edge of a canvas should scroll the
     * canvas (default is true).
     * @return if this tool wants mouse events to cause scrolling of canvas.
     */
    virtual bool wantsAutoScroll() const;

    /**
     * Called by the canvas to paint any decorations that the tool deems needed.
     * The painter has the top left of the canvas as its origin.
     * @param painter used for painting the shape
     * @param converter to convert between internal and view coordinates.
     */
    virtual void paint(QPainter &painter, const KoViewConverter &converter) = 0;

    /**
     * Return the option widgets for this tool. Create them if they
     * do not exist yet. If the tool does not have an option widget,
     * this method return an empty list. (After discussion with Thomas, who prefers
     * the toolmanager to handle that case.)
     *
     * @see m_optionWidgets
     */
    QMap<QString, QWidget *> optionWidgets();

    /**
     * Returns the internal selection option of this tool.
     * Each tool can have a selection which is private to that tool and the specified shape that it comes with.
     * The default returns 0.
     */
    virtual KoToolSelection *selection();

    /**
     * Retrieves the entire collection of actions for the tool.
     */
    QHash<QString, KAction*> actions(ReadWrite readWrite = ReadWriteAction) const;

    /**
     * Retrieve an action by name.
     */
    KAction *action(const QString &name) const;

    /**
     * Called when (one of) the mouse or stylus buttons is pressed.
     * Implementors should call event->ignore() if they do not actually use the event.
     * @param event state and reason of this mouse or stylus press
     */
    virtual void mousePressEvent(KoPointerEvent *event) = 0;

    /**
     * Called when (one of) the mouse or stylus buttons is double clicked.
     * Implementors should call event->ignore() if they do not actually use the event.
     * Default implementation ignores this event.
     * @param event state and reason of this mouse or stylus press
     */
    virtual void mouseDoubleClickEvent(KoPointerEvent *event);

    /**
     * Called when the mouse or stylus moved over the canvas.
     * Implementors should call event->ignore() if they do not actually use the event.
     * @param event state and reason of this mouse or stylus move
     */
    virtual void mouseMoveEvent(KoPointerEvent *event) = 0;

    /**
     * Called when (one of) the mouse or stylus buttons is released.
     * Implementors should call event->ignore() if they do not actually use the event.
     * @param event state and reason of this mouse or stylus release
     */
    virtual void mouseReleaseEvent(KoPointerEvent *event) = 0;

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

    /**
     * Called when the scrollwheel is used
     * Implementors should call event->ignore() if they do not actually use the event
     * @param event state of this wheel event
     */
    virtual void wheelEvent(KoPointerEvent *event);

    /**
     * This method is used to query a set of properties of the tool to be
     * able to support complex input method operations as support for surrounding
     * text and reconversions.
     * Default implementation returns simple defaults, for tools that want to provide
     * a more responsive text entry experience for CJK languages it would be good to reimplemnt.
     * @param query specifies which property is queried.
     * @param converter the view converter for the current canvas.
     */
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &converter) const;

    /**
     * Text entry of complex text, like CJK, can be made more interactive if a tool
     * implements this and the InputMethodQuery() methods.
     * Reimplementing this only provides the user with a more responsive text experience, since the
     * default implementation forwards the typed text as key pressed events.
     * @param event the input method event.
     */
    virtual void inputMethodEvent(QInputMethodEvent *event);

    /**
     * Called when (one of) a custom device buttons is pressed.
     * Implementors should call event->ignore() if they do not actually use the event.
     * @param event state and reason of this custom device press
     */
    virtual void customPressEvent(KoPointerEvent *event);

    /**
     * Called when (one of) a custom device buttons is released.
     * Implementors should call event->ignore() if they do not actually use the event.
     * @param event state and reason of this custom device release
     */
    virtual void customReleaseEvent(KoPointerEvent *event);

    /**
     * Called when a custom device moved over the canvas.
     * Implementors should call event->ignore() if they do not actually use the event.
     * @param event state and reason of this custom device move
     */
    virtual void customMoveEvent(KoPointerEvent *event);

    /**
     * Set the identifier code from the KoToolFactoryBase that created this tool.
     * @param id the identifier code
     * @see KoToolFactoryBase::id()
     */
    void setToolId(const QString &id);

    /**
     * get the identifier code from the KoToolFactoryBase that created this tool.
     * @return the toolId.
     * @see KoToolFactoryBase::id()
     */
    QString toolId() const;

    /// return the last emitted cursor
    QCursor cursor() const;

    /**
     * copies the tools selection to the clipboard.
     * The default implementation is empty to aid tools that don't have any selection.
     * @see selection()
     */
    virtual void copy() const;

    /**
     * Delete the tools selection.
     * The default implementation is empty to aid tools that don't have any selection.
     * @see selection()
     */
    virtual void deleteSelection();

    /**
     * Cut the tools selection and copy it to the clipboard.
     * The default implementation calls copy() and then deleteSelection()
     * @see copy()
     * @see deleteSelection()
     */
    virtual void cut();

    /**
     * Paste the clipboard selection.
     * A tool typically has one or more shapes selected and pasting should do something meaningful
     * for this specific shape and tool combination.  Inserting text in a text tool, for example.
     * If you reimplement this function make sure to also reimplement supportedPasteMimeTypes().
     * @return will return true if pasting succeeded. False if nothing happened.
     */
    virtual bool paste();

    /**
     * Returns the mimetypes that this tool's paste() function can handle
     * @return QStringList containing the mimetypes that's supported by paste()
     */
    virtual QStringList supportedPasteMimeTypes() const;

    /**
     * @return A list of actions to be used for a popup.
     */
    QList<QAction*> popupActionList() const;

    /// Returns the canvas the tool is working on
    KoCanvasBase *canvas() const;

    /**
     * Calling this will turn the tool into a read/write or a read-only tool.
     * Note that upon calling this method no actions will be enabled/disabled
     * as that is the responsibility of the KoToolManager. The KoToolManager
     * should use this variable at next tool switch.
     *
     * @param readWrite if true all available actions will be enabled, if false
     *   only actions that do not change the content will be available.
     * @see KoToolManager::updateReadWrite(), isReadWrite()
     */
    void setReadWrite(bool readWrite);

    /**
     * @return returns true if all available actions are be enabled, if false
     *   only actions that do not change the content are be available.
     * @see setReadWrite()
     */
    bool isReadWrite() const;

    /**
      * This method can be reimplemented in a subclass.
      * @return returns true, if the tool is in text mode. that means, that there is
      *   any kind of textual input and all single key shortcuts should be eaten.
      */
    virtual bool isInTextMode() const;

public slots:

    /**
     * This method is called when this tool instance is activated.
     * For any main window there is only one tool active at a time, which then gets all
     * user input.  Switching between tools will call deactivate on one and activate on the
     * new tool allowing the tool to flush items (like a selection)
     * when it is not in use.
     *
     * <p>There is one case where two tools are activated at the same.  This is the case
     * where one tool delegates work to another temporarily.  For example, while shift is
     * being held down.  The second tool will get activated with temporary=true and
     * it should emit done() when the state that activated it is ended.
     * <p>One of the important tasks of activate is to call useCursor()
     *
     * @param shapes the set of shapes that are selected or suggested for editing by a
     *      selected shape for the tool to work on.  Not all shapes will be meant for this
     *      tool.
     * @param toolActivation if TemporaryActivation, this tool is only temporarily actived
     *                  and should emit done when it is done.
     * @see deactivate()
     */
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) = 0;

    /**
     * This method is called whenever this tool is no longer the
     * active tool
     * @see activate()
     */
    virtual void deactivate();

    /**
     * This method is called whenever a property in the resource
     * provider associated with the canvas this tool belongs to
     * changes. An example is currently selected foreground color.
     */
    virtual void resourceChanged(int key, const QVariant &res);

    /**
     * This method just relays the given text via the tools statusTextChanged signal.
     * @param statusText the new status text
     */
    void setStatusText(const QString &statusText);

signals:

    /**
     * Emitted when this tool wants itself to be replaced by another tool.
     *
     * @param id the identification of the desired tool
     * @see toolId(), KoToolFactoryBase::id()
     */
    void activateTool(const QString &id);

    /**
     * Emitted when this tool wants itself to temporarily be replaced by another tool.
     * For instance, a paint tool could desire to be
     * temporarily replaced by a pan tool which could be temporarily
     * replaced by a colorpicker.
     * @param id the identification of the desired tool
     */
    void activateTemporary(const QString &id);

    /**
     * Emitted when the tool has been temporarily activated and wants
     * to notify the world that it's done.
     */
    void done();

    /**
     * Emitted by useCursor() when the cursor to display on the canvas is changed.
     * The KoToolManager should connect to this signal to handle cursors further.
     */
    void cursorChanged(const QCursor &cursor);

    /**
     * A tool can have a selection that is copy-able, this signal is emitted when that status changes.
     * @param hasSelection is true when the tool holds selected data.
     */
    void selectionChanged(bool hasSelection);

    /**
     * Emitted when the tool wants to display a different status text
     * @param statusText the new status text
     */
    void statusTextChanged(const QString &statusText);

protected:
    /**
     * Classes inheriting from this one can call this method to signify which cursor
     * the tool wants to display at this time.  Logical place to call it is after an
     * incoming event has been handled.
     * @param cursor the new cursor.
     */
    void useCursor(const QCursor &cursor);

    /**
     * Reimplement this if your tool actually has an option widget.
     * Sets the option widget to 0 by default.
     */
    virtual QWidget *createOptionWidget();
    virtual QMap<QString, QWidget *> createOptionWidgets();

    /**
     * Add an action under the given name to the collection.
     *
     * Inserting an action under a name that is already used for another action will replace
     * the other action in the collection.
     *
     * @param name The name by which the action be retrieved again from the collection.
     * @param action The action to add.
     * @param readWrite set this to ReadOnlyAction to keep the action available on
     *      read-only documents
     */
    void addAction(const QString &name, KAction *action, ReadWrite readWrite = ReadWriteAction);

    /**
     * Set the list of actions to be used as popup menu.
     * @param list the list of actions.
     * @see popupActionList
     */
    void setPopupActionList(const QList<QAction*> &list);

    /**
    * Returns a handle grab rect at the given position.
    *
    * The position is expected to be in document coordinates. The grab sensitivity
    * canvas resource is used for the dimension of the rectangle.
    *
    * @return the handle rectangle in document coordinates
    */
    QRectF handleGrabRect(const QPointF &position) const;

    /**
    * Returns a handle paint rect at the given position.
    *
    * The position is expected to be in document coordinates. The handle radius
    * canvas resource is used for the dimension of the rectangle.
    *
    * @return the handle rectangle in document coordinates
    */
    QRectF handlePaintRect(const QPointF &position) const;

protected:
    KoToolBase(KoToolBasePrivate &dd);

    KoToolBasePrivate *d_ptr;

private:
    KoToolBase();
    KoToolBase(const KoToolBase&);
    KoToolBase& operator=(const KoToolBase&);

    Q_DECLARE_PRIVATE(KoToolBase)
};

#endif /* KOTOOL_H */
