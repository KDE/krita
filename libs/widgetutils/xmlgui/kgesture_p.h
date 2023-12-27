/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Andreas Hartmetz (ahartmetz@gmail.com)

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KGESTURE_H
#define KGESTURE_H

#include <kritawidgetutils_export.h>

#include <QString>
#include <QHash>
#include <QPolygon>

/*
 kinds of gestures:
 -shapes like triangle, right angle, line
 -"rocker" (i.e. two mouse button) gestures
 */

class KisKShapeGesturePrivate;
//TODO: implement operator== for special situations like in KKeyChooser.
class KRITAWIDGETUTILS_EXPORT KisKShapeGesture
{
public:
    /**
     * Create a new invalid shape gesture.
     */
    KisKShapeGesture();

    /**
     * Creates a new gesture consisting of given shape.
     * If the gesture belongs to a KAction, and the user draws approximately the same shape
     * on the screen while holding down the right mouse button, the action will trigger.
     * @p shape must be a "reasonable" polygon. It must contain at least two points
     * and it should contain at most 50 for performance reasons. No two consecutive points
     * are allowed to be at the same position.
     * @param shape to draw to trigger this gesture
     */
    KisKShapeGesture(const QPolygon &shape);

    /**
     * Creates a new gesture from a string description.
     * @param description create gesture according to this
     */
    KisKShapeGesture(const QString &description);

    /**
     * Copies the given gesture.
     * @param other gesture to copy
     */
    KisKShapeGesture(const KisKShapeGesture &other);

    /**
     * Destructor.
     */
    ~KisKShapeGesture();

    /**
     * Set the shape to draw to trigger this gesture.
     */
    void setShape(const QPolygon &shape);

    /**
     * set a user-visible name for this gesture's shape, like "triangle" or "line".
     */
    void setShapeName(const QString &friendlyName);

    /**
     * Return the user-visible name for this gesture's shape, like "triangle" or "line".
     */
    QString shapeName() const;

    /**
     * Return true if this gesture is valid.
     *
     */
    bool isValid() const;

    /**
     * Return a string representation of this gesture.
     * Return empty string if invalid.
     * This function is mainly for use with config files.
     *
     * @see shapeName()
     */
    QString toString() const;

    /**
     * Return an idealized SVG image of this gesture.
     * Return an empty image if invalid.
     * @param attributes SVG attributes to apply to the SVG "path" element that
     * makes up the drawing of the gesture. By default, only a 'fill="none"'
     * attribute will be set.
     */
    QByteArray toSvg(const QString &attributes = QString()) const;

    /**
     * Return a difference measurement between this gesture and the @p other
     * gesture. Abort comparison if difference is larger than @p abortThreshold
     * and return a very large difference in that case.
     * Usual return values range from x to y //TODO: fill in x and y
     */
    float distance(const KisKShapeGesture &other, float abortThreshold) const;

    /**
     * Set this gesture to the other gesture.
     */
    KisKShapeGesture &operator=(const KisKShapeGesture &other);

    /**
     * Return whether this gesture is equal to the other gesture.
     */
    bool operator==(const KisKShapeGesture &other) const;

    /**
     * Return the opposite of operator==()
     */
    bool operator!=(const KisKShapeGesture &other) const;

    /**
     * Return an opaque value for use in hash tables
     */
    uint hashable() const;

private:
    KisKShapeGesturePrivate *const d;
};

inline uint qHash(const KisKShapeGesture &key)
{
    return qHash(key.hashable());
}

class KisKRockerGesturePrivate;

class KRITAWIDGETUTILS_EXPORT KisKRockerGesture
{
public:
    /**
     * Create a new invalid rocker gesture.
     */
    KisKRockerGesture();

    /**
     * Creates a new gesture consisting of given buttons.
     * @param hold create gesture according to this hold
     * @param thenPush create gesture according to this push
     */
    KisKRockerGesture(enum Qt::MouseButton hold, enum Qt::MouseButton thenPush);

    /**
     * Creates a new gesture from a string description.
     * @param description create gesture according to this
     */
    KisKRockerGesture(const QString &description);

    /**
     * Copies the given gesture.
     * @param other gesture to copy
     */
    KisKRockerGesture(const KisKRockerGesture &other);

    /**
     * Destructor.
     */
    ~KisKRockerGesture();

    /**
     * set button combination to trigger
     */
    void setButtons(Qt::MouseButton hold, Qt::MouseButton thenPush);

    /**
     * Write the button combination to hold and thenPush
     */
    void getButtons(Qt::MouseButton *hold, Qt::MouseButton *thenPush) const;

    /**
     * Return a user-friendly name of the button combination.
     */
    QString rockerName() const;

    /**
     * Return a user-friendly name for the mouse button button
     */
    static QString mouseButtonName(Qt::MouseButton button);

    /**
     * Return true if this gesture is valid.
     */
    bool isValid() const;

    /**
     * Return a string representation of this gesture.
     * Return an empty string if invalid.
     * This function is mainly for use with config files.
     *
     * @see rockerName()
     */
    QString toString() const;

    /**
     * Set this gesture to the other gesture.
     */
    KisKRockerGesture &operator=(const KisKRockerGesture &other);

    /**
     * Return whether this gesture is equal to the other gesture.
     */
    bool operator==(const KisKRockerGesture &other) const;

    /**
     * Return the opposite of operator==()
     */
    bool operator!=(const KisKRockerGesture &other) const;

    /**
     * Return an opaque value for use in hash tables
     */
    uint hashable() const;

private:
    KisKRockerGesturePrivate *const d;
};

inline uint qHash(const KisKRockerGesture &key)
{
    return qHash(key.hashable());
}

//KGESTURE_H
#endif
