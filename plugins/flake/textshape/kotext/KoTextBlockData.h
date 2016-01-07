/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Boudewijn Rempt <boud@kogmbh.com>
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

#ifndef KOTEXTBLOCKDATA_H
#define KOTEXTBLOCKDATA_H

#include <QTextBlockUserData>

#include "kritatext_export.h"

class KoTextBlockBorderData;
class KoTextBlockPaintStrategyBase;

/**
 * This class is used to store properties for KoText layouting inside Qt QTextBlock
 * instances.
 */
class KRITATEXT_EXPORT KoTextBlockData
{
public:
    /**
     * Supplemental data to allow advanced tabs to be used for layout and painting.
     * Qt-Scribe knows only left-tabs and it also only knows tab-positions per paragraph
     * which is not enough for our needs.
     * Using the tabs list we calculated in the layout step, we can emulate
     * all tabs by setting these as left-tabs on scribe prior to the re-layout of the text in the
     * line which is redone at painting time.
     * The tabLength list holds a length for each tab in the line and thus corresponds to the tab
     * positions in the tabs list.  We can then calculate the tab to have started the position
     * minus the length and use that to paint special tab attributes.
     */
    struct TabLineData {
        /// the tab positions as set on the QTextOption.setTabArray()
        QList<qreal> tabs;
        /// the length of each tab so we know which area to paint when we want to decorate it.
        QList<qreal> tabLength;
    };

    /**
     * Datastructure to define a range of characters assigned a temporary meaning.
     * Common use cases are spellchecking and grammar.
     */
    struct MarkupRange {
        int firstChar;
        int lastChar;
        qreal startX;
        qreal endX;
    };

    /**
     * The different types of markups.
     */
    enum MarkupType {
        Misspell,
        Grammar
    };

    explicit KoTextBlockData(QTextBlock &block);
    explicit KoTextBlockData(QTextBlockUserData *userData);
    virtual ~KoTextBlockData();

    /**
     * Add a range to the _end_ of the list of markups. It's important that firstChar is after
     * lastChar of the previous range for that type of markup.
     */
    void appendMarkup(MarkupType type, int firstChar, int lastChar);

    /**
     * Clear all ranges for a specific type of markup.
     */
    void clearMarkups(MarkupType type);

    /**
     * Move all ranges following fromPosition delta number of characters to the right.
     * Applies to a specific type of markup.
     */
    void rebaseMarkups(MarkupType type, int fromPosition, int delta);

    /**
     * Find a range that contains positionWithin.
     * If none is found a default Markuprange firstChar = lastChar = 0 is returned
     */
    MarkupRange findMarkup(MarkupType type, int positionWithin) const;

    void setMarkupsLayoutValidity(MarkupType type, bool valid);
    bool isMarkupsLayoutValid(MarkupType type) const;

    QList<MarkupRange>::Iterator markupsBegin(MarkupType type);
    QList<MarkupRange>::Iterator markupsEnd(MarkupType type);

    /**
     * Clear the counter and set everything to default values.
     */
    void clearCounter();

    /// return if this block has up-to-date counter data
    bool hasCounterData() const;
    /// return the width (in pt) of the counter.
    qreal counterWidth() const;
    /// set the width of the counter in pt.
    void setCounterWidth(qreal width);
    /// return the spacing (in pt) between the counter and the text
    qreal counterSpacing() const;
    /// set the spacing (in pt) between the counter and the text
    void setCounterSpacing(qreal spacing);

    /** sets the index that is used at this level.
     * If this represents a paragraph with counter 3.1, then the text is the 1.
     * If this represents a paragraph with counter IV.V, then the index is 5.
     */
    void setCounterIndex(int index);
    /// returns the index for the counter at this level
    int counterIndex() const;

    /// return the exact text that will be painted as the counter
    QString counterText() const;

    /**
     * set the text that is used for the counter at this level. the text is formatted
     * depending on the language/style.
     * If this represents a parag with counter 3.1 then the text is the '1'..
     * If this represents a paragraph with counter IV.V, then the text is V.
     * since the rest is not dependent on this parag, but only its location in the text
     *
     */
    void setPartialCounterText(const QString &text);
    /// return the partial text for this paragraphs counter
    QString partialCounterText() const;

    /// set the plain counter text which equals the counterText minus prefix and sufix
    void setCounterPlainText(const QString &text);
    /// return the plain counter text which equals the counterText minus prefix and sufix
    QString counterPlainText() const;

    void setCounterPrefix(const QString &text);
    QString counterPrefix() const;

    void setCounterSuffix(const QString &text);
    QString counterSuffix() const;

    /// Set if the counter is a image or not
    void setCounterIsImage(bool isImage);

    /// return if the counter is a image or not
    bool counterIsImage() const;

    /**
     * The actual position of the counter can be set, in actual (text) document coordinates.
     * @param position the location of the top/left of the counter text line.
     */
    void setCounterPosition(const QPointF &position);
    /**
     * Return the counter position.
     * @see setCounterPosition
     */
    QPointF counterPosition() const;

    /**
     * Sets a textformat to be used for the counter/bullet
     * @param font the format
     */
    void setLabelFormat(const QTextCharFormat &format);

    /**
     * Return the format to be used for the counter/bullet
     */
    QTextCharFormat labelFormat() const;

    /**
     * When a paragraph has a border, it will have a KoTextBlockBorderData instance.
     * Adding the border will increase the refcount.
     * @param border the border used for this paragraph, or 0 if no border is needed (anymore).
     */
    void setBorder(KoTextBlockBorderData *border);

    /**
     * Return the border associated with this paragraph, or 0 if there is no border set.
     */
    KoTextBlockBorderData *border() const;

    /**
     * sets a paintStrategy of this paragraph
     * @param paintStrategy the paintStrategy to be used for this paragraph
     */
    void setPaintStrategy(KoTextBlockPaintStrategyBase *paintStrategy);

    /**
     * Return the paintStrategy of this paragraph
     */
    KoTextBlockPaintStrategyBase *paintStrategy() const;

    /**
     * @brief saveXmlID can be used to determine whether we need to save the xml:id
     *    for this text block data object. This is true if the text block data describes
     *    animations.
     * @return true of we need to save the xml id, false if not.
     */
    bool saveXmlID() const;

private:
    class Private;
    Private * const d;
};

Q_DECLARE_METATYPE(QTextBlockUserData*)

#endif
