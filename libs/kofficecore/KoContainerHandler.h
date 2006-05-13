/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>

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

#ifndef HANDLER_H
#define HANDLER_H

#include <QObject>
//Added by qt3to4:
#include <QMouseEvent>
#include <QEvent>
#include <KoDocumentChild.h>

class QMatrix;

class KoView;
class KoPartResizeHandlerPrivate;
class KoPartMoveHandlerPrivate;

/**
 * @brief An abstract base class for event handlers.
 *
 * The idea of an event handler is that it is created for a
 * certain purpose, for example moving or resizing of a part.
 * Once that action is finished, the handler will destroy
 * itself.
 *
 * This design pattern helps you to keep your event filters
 * and your mousePressEvent, mouseMoveEvent etc. methods clean.
 */
class KOFFICECORE_EXPORT KoEventHandler : public QObject
{
    Q_OBJECT
public:
    KoEventHandler( QObject* target );
    ~KoEventHandler();

    QObject* target();

private:
    QObject* m_target;
};

/**
 * Used by @ref KoContainerHandler internally to handle resizing of
 * embedded documents.
 */
class KoPartResizeHandler : public KoEventHandler
{
    Q_OBJECT
public:
    KoPartResizeHandler( QWidget* widget, const QMatrix& matrix, KoView* view, KoChild* child,
                       KoChild::Gadget gadget, const QPoint& point );
    ~KoPartResizeHandler();

protected:
    void repaint(QRegion &rgn);
    bool eventFilter( QObject*, QEvent* );

private:
    KoPartResizeHandlerPrivate *d;
};

/**
 * Used by @ref KoContainerHandler internally to handle moving of
 * embedded documents.
 */
class KoPartMoveHandler : public KoEventHandler
{
    Q_OBJECT
public:
    KoPartMoveHandler( QWidget* widget, const QMatrix& matrix, KoView* view, KoChild* child,
                     const QPoint& point );
    ~KoPartMoveHandler();

protected:
    bool eventFilter( QObject*, QEvent* );

private:
    KoPartMoveHandlerPrivate *d;
};

/**
 * This class can handle moving and resizing of embedded
 * documents in your class derived from @ref KoView.
 *
 * Just create one instance per view of this class and parts
 * will magically be moved around on your document.
 *
 * This class acts like an event filter on your view, so the
 * mouse events which are used for parts moving and resizing
 * will never show up in your view.
 *
 * @see KoPartMoveHandlerPrivate
 * @see KoPartResizeHandlerPrivate
 */
class KOFFICECORE_EXPORT KoContainerHandler : public KoEventHandler
{
    Q_OBJECT
public:
    KoContainerHandler( KoView* view, QWidget* widget );
    ~KoContainerHandler();

signals:
    /**
     * Emitted if the user wants to open the popup menu for some
     * child object.
     */
    void popupMenu( KoChild*, const QPoint& global_pos );

    /**
      * Emitted if the user pressed the delete key whilst a child was selected
      */
    void deleteChild( KoChild* );

protected:
    bool eventFilter( QObject*, QEvent* );

private:
    /// This is a little helper function to get rid of some duplicated code
    KoChild *child(KoChild::Gadget &gadget, QPoint &pos, const QMouseEvent *ev);
    KoView* m_view;
};

#endif
