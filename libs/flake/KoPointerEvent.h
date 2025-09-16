/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2006 C. Boemann Rasmussen <cbo@boemann.dk>
   SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOPOINTEREVENT_H
#define KOPOINTEREVENT_H

#include <QTouchEvent>
#include <QSharedPointer>

class QTabletEvent;
class QMouseEvent;
class QWheelEvent;

#include "kritaflake_export.h"

struct KoPointerEventWrapper;

/**
 * KoPointerEvent is a synthetic event that can be built from a mouse,
 * touch or tablet event. In addition to always providing tools with tablet
 * pressure characteristics, KoPointerEvent has both the original
 * (canvas based) position as well as the normalized position, that is,
 * the position of the event _in_ the document coordinates.
 */
class KRITAFLAKE_EXPORT KoPointerEvent
{
public:
    /**
     * Constructor.
     *
     * @param event the mouse event that is the base of this event.
     * @param point the zoomed point in the normal coordinate system.
     */
    KoPointerEvent(QMouseEvent *event, const QPointF &point);

    /**
     * Constructor.
     *
     * @param event the tablet event that is the base of this event.
     * @param point the zoomed point in the normal coordinate system.
     */
    KoPointerEvent(QTabletEvent *event, const QPointF &point);

    KoPointerEvent(QTouchEvent* ev, const QPointF& pnt);

    KoPointerEvent(KoPointerEvent *event, const QPointF& point);

    ~KoPointerEvent();

    /**
     * Copies the event object
     *
     * The newly created object will still point to the original
     * QMouseEvent, QTabletEvent or QTouchEvent, so it is not
     * safe to store such object. If you want to store a KoPointerEvent
     * object, use deepCopyEvent() instead.
     */
    KoPointerEvent(const KoPointerEvent &rhs);

    /**
     * Copies the event object
     *
     * See a comment in copy constructor for the difference between
     * deep/shallow copies.
     */
    KoPointerEvent& operator=(const KoPointerEvent &rhs);

    /**
     * Copies KoPointerEvent **and** its underlying Qt event.
     *
     * Normal copy-constructor keeps the pointers to the original
     * Qt event intact, therefore you cannot store this event for
     * any time longer than the lifetime of the handler for this event.
     */
    KoPointerEventWrapper deepCopyEvent() const;

    /**
     * For classes that are handed this event, you can choose to accept (default) this event.
     * Acceptance signifies that you have handled this event and found it useful, the effect
     * of that will be that the event will not be handled to other event handlers.
     */
    void accept();

    /**
     * For classes that are handed this event, you can choose to ignore this event.
     * Ignoring this event means you have not handled it and want to allow other event
     * handlers to try to handle it.
     */
    void ignore();

    /**
     * Returns the keyboard modifier flags that existed immediately before the event occurred.
     * See also QApplication::keyboardModifiers().
     */
    Qt::KeyboardModifiers modifiers() const;

    /// return if the event has been accepted.
    bool isAccepted() const;

    /// return if this event was spontaneous (see QMouseEvent::spontaneous())
    bool spontaneous() const;

    /// return button pressed (see QMouseEvent::button());
    Qt::MouseButton button() const;

    /// return buttons pressed (see QMouseEvent::buttons());
    Qt::MouseButtons buttons() const;

    /// Return the position screen coordinates
    QPoint globalPos() const;

    /// return the position in widget coordinates
    QPoint pos() const;

    /**
     * return the pressure (or a default value). The range is 0.0 - 1.0
     * and the default pressure (this is the pressure that will be given
     * when you use something like the mouse) is 1.0
     */
    qreal pressure() const;

    /// return the rotation (or a default value)
    qreal rotation() const;

    /**
     * return the tangential pressure  (or a default value)
     * This is typically given by a finger wheel on an airbrush tool. The range
     * is from -1.0 to 1.0. 0.0 indicates a neutral position. Current airbrushes can
     * only move in the positive direction from the neutral position. If the device
     * does not support tangential pressure, this value is always 0.0.
     */
    qreal tangentialPressure() const;

    /**
     * Return the x position in widget coordinates.
     * @see point
     */
    int x() const;

    /**
     * Returns the angle between the device (a pen, for example) and the
     * perpendicular in the direction of the x axis. Positive values are
     * towards the tablet's physical right. The angle is in the range -60
     * to +60 degrees. The default value is 0.
     */
    int xTilt() const;

    /**
     * Return the y position in widget coordinates.
     * @see point
     */
    int y() const;

    /**
     * Returns the angle between the device (a pen, for example) and the
     * perpendicular in the direction of the x axis. Positive values are
     * towards the tablet's physical right. The angle is in the range -60
     * to +60 degrees. The default value is 0.
     */
    int yTilt() const;

    /**
     * Returns the z position of the device. Typically this is represented
     * by a wheel on a 4D Mouse. If the device does not support a Z-axis,
     * this value is always zero. This is <em>not</em> the same as pressure.
     */
    int z() const;

    /**
     * Returns the time the event was registered.
     */
    ulong time() const;


    /// The point in document coordinates.
    QPointF point;

    /**
     * Returns if the event comes from a tablet
     */
    bool isTabletEvent() const;

    /**
     * Returns if the event comes from a touch
     */
    bool isTouchEvent() const;

    /**
     * Whether we ever had any tablet inputs this session
     */
    static bool tabletInputReceived();

public:
    static void copyQtPointerEvent(const QMouseEvent *event, QScopedPointer<QEvent> &dst);
    static void copyQtPointerEvent(const QTabletEvent *event, QScopedPointer<QEvent> &dst);
    static void copyQtPointerEvent(const QTouchEvent *event, QScopedPointer<QEvent> &dst);

protected:
    friend class KoToolProxy;
    friend class KisToolProxy;
    friend class KisScratchPadEventFilter;
private:

    class Private;
    const QScopedPointer<Private> d;
};

struct KRITAFLAKE_EXPORT KoPointerEventWrapper
{
    template <typename Event>
    KoPointerEventWrapper(Event *_event, const QPointF &point);

    KoPointerEvent event;
    QSharedPointer<QEvent> baseQtEvent;
};

#endif

