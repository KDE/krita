/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
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

#ifndef KOTEXTBLOCKDATA_H
#define KOTEXTBLOCKDATA_H

#include <QTextBlockUserData>

#include <kotext_export.h>

class KoTextBlockBorderData;

/**
 * This class is used to store properties for KoText layouting inside Qt QTextBlock
 * instances.
 */
class KOTEXT_EXPORT KoTextBlockData : public QTextBlockUserData {
public:
    KoTextBlockData();
    ~KoTextBlockData();

    /// return if this block has up-to-date counter data
    bool hasCounterData() const;
    /// return the width (in pt) of the counter.
    double counterWidth() const;
    /// set the width of the counter in pt.
    void setCounterWidth(double width);
    /// return the spacing (in pt) between the counter and the text
    double counterSpacing() const;
    /// set the spacing (in pt) between the counter and the text
    void setCounterSpacing(double spacing);
    /// set the exact text that will be painted as the counter
    void setCounterText(const QString &text);
    /// return the exact text that will be painted as the counter
    const QString &counterText() const;

    /**
     * set the text that is used for the counter at this level.
     * If this represents a parag with counter 3.1 then the text is the '1'
     * since the rest is not dependent on this parag, but only its location in the text
     */
    void setPartialCounterText(const QString &text);
    /// return the partial text for this paragraphs counter
    const QString &partialCounterText() const;

    /**
     * The actual position of the counter can be set, in actual (text) document coordinates.
     * @param position the location of the top/left of the counter text line.
     */
    void setCounterPosition(QPointF position);
    /**
     * Return the counter position.
     * @see setCounterPosition
     */
    const QPointF &counterPosition() const;

    /**
     * When a paragraph has a border, it will have a KoTextBlockBorderData instance.
     * Adding the border will increase the refcount.
     * @param the border used for this paragraph, or 0 if no border is needed (anymore).
     */
    void setBorder(KoTextBlockBorderData *border);
    /**
     * Return the border associated with this paragraph, or 0 if there is no border set.
     */
    KoTextBlockBorderData *border() const;

private:
    class Private;
    Private * const d;
};

#endif
