/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KOFILTEREFFECTSTACK
#define KOFILTEREFFECTSTACK

#include "kritaflake_export.h"

#include <QList>

class KoFilterEffect;
class KoXmlWriter;

class QRectF;

/// This class manages a stack of filter effects
class KRITAFLAKE_EXPORT KoFilterEffectStack
{
public:
    /// Creates an empty filter effect stack
    KoFilterEffectStack();

    /// Destroys the filter effect stack, deleting all filter effects
    ~KoFilterEffectStack();

    /**
    * The first filter of the list is the first to be applied.
    *
    * @return the list of filter effects applied on the shape when rendering.
    */
    QList<KoFilterEffect*> filterEffects() const;

    /**
    * Returns if the filter effect stack is empty.
    * @return false if the stack contains filter effects, otherwise true
    */
    bool isEmpty() const;

    /**
    * Inserts a new filter at the given position in the filter list.
    *
    * The filter stack take ownership of the inserted filter effect.
    *
    * @param index the list index to insert the new filter at
    * @param filter the new filter to insert
    */
    void insertFilterEffect(int index, KoFilterEffect *filter);

    /**
    * Appends a new filter at the end of the filter list.
    *
    * The filter stack take ownership of the appended filter effect.
    *
    * @param filter the new filter to append
    */
    void appendFilterEffect(KoFilterEffect *filter);

    /**
    * Removes the filter with the given index from the filter list.
    *
    * The filter gets deleted after removal from the list.
    *
    * @param index the index of the filter to remove
    */
    void removeFilterEffect(int index);

    /**
     * Take filter effect with given index from the stack and returns it.
     * @param index the index of the filter to take
     * @return the filter effect, of 0 if no filter effect with the given index exists
     */
    KoFilterEffect* takeFilterEffect(int index);

    /// Sets the clipping rectangle used for this filter in bounding box units
    void setClipRect(const QRectF &clipRect);

    /// Returns the clipping rectangle used for this filter in bounding box units
    QRectF clipRect() const;

    /// Returns the clipping rectangle for the given bounding rect
    QRectF clipRectForBoundingRect(const QRectF &boundingRect) const;

    /**
     * Increments the use-value.
     * Returns true if the new value is non-zero, false otherwise.
     */
    bool ref();

    /**
     * Decrements the use-value.
     * Returns true if the new value is non-zero, false otherwise.
     */
    bool deref();

    /// Return reference counter
    int useCount() const;

    /**
    * Saves filter stack using given xml writer.
    * @param writer the xml writer to write data to
    * @param id the filter id to write, used for referencing the filter
    */
    void save(KoXmlWriter &writer, const QString &filterId);

    /// Returns list of required standard inputs
    QSet<QString> requiredStandarsInputs() const;
private:
    class Private;
    Private * const d;
};

#endif // KOFILTEREFFECTSTACK
