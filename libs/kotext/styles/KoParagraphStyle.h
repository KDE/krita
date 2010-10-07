/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007,2008 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007,ducroquet Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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
#ifndef KOPARAGRAPHSTYLE_H
#define KOPARAGRAPHSTYLE_H

#include "KoText.h"
#include "kotext_export.h"

#include <QObject>
#include <QVector>
#include <QString>
#include <QVariant>
#include <QTextFormat>

struct Property;
class KoCharacterStyle;
class KoListStyle;
class QTextBlock;
class KoStyleStack;
class KoGenStyle;
class KoGenStyles;
#include "KoXmlReaderForward.h"
class KoShapeLoadingContext;
#include "KoBorder.h"

/**
 * A container for all properties for the paragraph wide style.
 * Each paragraph in the main text either is based on a parag style, or its not. Where
 * it is based on a paragraph style this is indecated that it has a property 'StyleId'
 * with an integer as value.  The integer value corresponds to the styleId() output of
 * a specific KoParagraphStyle.
 * @see KoStyleManager
 */
class KOTEXT_EXPORT KoParagraphStyle : public QObject
{
    Q_OBJECT
public:
    enum Property {
        StyleId = QTextFormat::UserProperty + 1,
        // Linespacing properties
        PercentLineHeight,  ///< this propery is used for a percentage of the highest character on that line
        FixedLineHeight,    ///< this propery is used to use a non-default line height
        MinimumLineHeight,  ///< this property is used to have a minimum line spacing
        LineSpacing,        ///< Hard leader height.
        LineSpacingFromFont,  ///< if false, use fontsize (in pt) solely, otherwise respect font settings
        AlignLastLine,      ///< When the paragraph is justified, what to do with the last word line
        WidowThreshold,     ///< If 'keep together'=false, amount of lines to keep it anyway.
        OrphanThreshold,   ///< If 'keep together'=false, amount of lines to keep it anyway.
        DropCaps,       ///< defines if a paragraph renders its first char(s) with drop-caps
        DropCapsLength, ///< Number of glyphs to show as drop-caps
        DropCapsLines,  ///< Number of lines that the drop-caps span
        DropCapsDistance,   ///< Distance between drop caps and text
        DropCapsTextStyle,  ///< Text style of dropped chars.
        FollowDocBaseline,  ///< If true the baselines will be aligned with the doc-wide grid

        // border stuff
        HasLeftBorder,  ///< If true, paint a border on the left
        HasTopBorder,   ///< If true, paint a border on the top
        HasRightBorder, ///< If true, paint a border on the right
        HasBottomBorder,///< If true, paint a border on the bottom
        BorderLineWidth,///< Thickness of inner-border
        SecondBorderLineWidth,  ///< Thickness of outer-border
        DistanceToSecondBorder, ///< Distance between inner and outer border
        LeftPadding,    ///< distance between text and border
        TopPadding,     ///< distance between text and border
        RightPadding,   ///< distance between text and border
        BottomPadding,   ///< distance between text and border
        LeftBorderWidth,        ///< The thickness of the border, or 0 if there is no border
        LeftInnerBorderWidth,   ///< In case of style being 'double' the thickness of the inner border line
        LeftBorderSpacing,      ///< In case of style being 'double' the space between the inner and outer border lines
        LeftBorderStyle,        ///< The border style. (see BorderStyle)
        LeftBorderColor,        ///< The border Color
        TopBorderWidth,         ///< The thickness of the border, or 0 if there is no border
        TopInnerBorderWidth,    ///< In case of style being 'double' the thickness of the inner border line
        TopBorderSpacing,       ///< In case of style being 'double' the space between the inner and outer border lines
        TopBorderStyle,         ///< The border style. (see BorderStyle)
        TopBorderColor,         ///< The border Color
        RightBorderWidth,       ///< The thickness of the border, or 0 if there is no border
        RightInnerBorderWidth,  ///< In case of style being 'double' the thickness of the inner border line
        RightBorderSpacing,     ///< In case of style being 'double' the space between the inner and outer border lines
        RightBorderStyle,       ///< The border style. (see BorderStyle)
        RightBorderColor,       ///< The border Color
        BottomBorderWidth,      ///< The thickness of the border, or 0 if there is no border
        BottomInnerBorderWidth, ///< In case of style being 'double' the thickness of the inner border line
        BottomBorderSpacing,    ///< In case of style being 'double' the space between the inner and outer border lines
        BottomBorderStyle,      ///< The border style. (see BorderStyle)
        BottomBorderColor,      ///< The border Color

        // lists
        ListStyleId,            ///< Style Id of associated list style
        ListStartValue,         ///< Int with the list-value that that parag will have. Ignored if this is not a list.
        RestartListNumbering,   ///< boolean to indicate that this paragraph will have numbering restart at the list-start. Ignored if this is not a list.
        ListLevel,               ///< int with the list-level that the paragraph will get when this is a list (numbered paragraphs)
        IsListHeader,           ///< bool, if true the paragraph shows up as a list item, but w/o a list label.
        UnnumberedListItem,     ///< bool. if true this paragraph is part of a list but is not numbered

        AutoTextIndent,         ///< bool, says whether the paragraph is auto-indented or not

        TabStopDistance,        ///< Double, Length. specifies that there's a tab stop every n inches
        ///< (after the last of the TabPositions, if any)
        TabPositions,           ///< A list of tab positions
        TextProgressionDirection,

        MasterPageName,         ///< Optional name of the master-page

        OutlineLevel,            ///< Outline level for headings
        DefaultOutlineLevel,

        // numbering
        LineNumbering,           ///< bool, specifies whether lines should be numbered in this paragraph
        LineNumberStartValue     ///< integer value that specifies the number for the first line in the paragraph

// do 15.5.24
// continue at 15.5.28
    };

    /// Constructor
    KoParagraphStyle(QObject *parent = 0);
    /// Creates a KoParagrahStyle with the given block format, the block character format and \a parent
    KoParagraphStyle(const QTextBlockFormat &blockFormat, const QTextCharFormat &blockCharFormat, QObject *parent = 0);
    /// Destructor
    ~KoParagraphStyle();

    /// Creates a KoParagraphStyle that represents the formatting of \a block.
    static KoParagraphStyle *fromBlock(const QTextBlock &block, QObject *parent = 0);

    /// creates a clone of this style with the specified parent
    KoParagraphStyle *clone(QObject *parent = 0);

    //  ***** Linespacing
    /**
     * Sets the line height as a percentage of the highest character on that line.
     * A good typographically correct value would be 120%
     * Note that lineSpacing() is added to this.
     * You should consider doing a remove(KoParagraphStyle::LineSpacing); because if set, it will
     *  be used instead of this value.
     * @see setLineSpacingFromFont
     */
    void setLineHeightPercent(int lineHeight);
    /// @see setLineHeightPercent
    int lineHeightPercent() const;

    /**
     * Sets the line height to a specific pt-based height, ignoring the font size.
     * Setting this will ignore the lineHeightPercent() and lineSpacing() values.
     */
    void setLineHeightAbsolute(qreal height);
    /// @see setLineHeightAbsolute
    qreal lineHeightAbsolute() const;

    /**
     * Sets the line height to have a minimum height in pt.
     * You should consider doing a remove(KoParagraphStyle::FixedLineHeight); because if set, it will
     *  be used instead of this value.
     */
    void setMinimumLineHeight(qreal height);
    /// @see setMinimumLineHeight
    qreal minimumLineHeight() const;

    /**
     * Sets the space between two lines to be a specific height. The total linespacing will become
     * the line height + this height.  Where the line height is dependent on the font.
     * You should consider doing a remove(KoParagraphStyle::FixedLineHeight) and a
     * remove(KoParagraphStyle::PercentLineHeight); because if set, they will be used instead of this value.
     */
    void setLineSpacing(qreal spacing);
    /// @see setLineSpacing
    qreal lineSpacing() const;

    /**
     * If set to true the font-encoded height will be used instead of the font-size propery
     * This property influences setLineHeightPercent() behavior.
     * When off (default) a font of 12pt will always have a linespacing of 12pt times the
     * current linespacing percentage.  When on the linespacing embedded in the font
     * is used which can differ for various fonts, even if they are the same pt-size.
     */
    void setLineSpacingFromFont(bool on);
    /**
     * @see setLineSpacingFromFont
     */
    bool lineSpacingFromFont() const;


    /**
     * For paragraphs that are justified the last line alignment is specified here.
     * There are only 3 valid options, Left, Center and Justified. (where Left will
     * be right aligned for RTL text).
     */
    void setAlignLastLine(Qt::Alignment alignment);
    /**
     * @see setAlignLastLine
     */
    Qt::Alignment alignLastLine() const;
    /**
     * Paragraphs that are broken across two frames are normally broken at the bottom
     * of the frame.  Using this property we can set the minimum number of lines that should
     * appear in the second frame to avoid really short paragraphs standing alone (also called
     * widows).  So, if a 10 line parag is broken in a way that only one line is in the second
     * frame, setting a widowThreshold of 4 will break at 6 lines instead to leave the
     * requested 4 lines.
     */
    void setWidowThreshold(int lines);
    /**
     * @see setWidowThreshold
     */
    int widowThreshold() const;
    /**
     * Paragraphs that are broken across two frames are normally broken at the bottom
     * of the frame.  Using this property we can set the minimum number of lines that should
     * appear in the first frame to avoid really short paragraphs standing alone (also called
     * orphans).  So, if a paragraph is broken so only 2 line is left in the first frame
     * setting the orphanThreshold to something greater than 2 will move the whole paragraph
     * to the second frame.
     */
    void setOrphanThreshold(int lines);
    /**
     * @see setOrphanThreshold
     */
    int orphanThreshold() const;
    /**
     * If true, make the first character span multiple lines.
     * @see setDropCapsLength
     * @see setDropCapsLines
     * @see dropCapsDistance
     */
    void setDropCaps(bool on);
    /**
     * @see setDropCaps
     */
    bool dropCaps() const;
    /**
     * Set the number of glyphs to show as drop-caps
     * @see setDropCaps
     * @see setDropCapsLines
     * @see dropCapsDistance
     */
    void setDropCapsLength(int characters);
    /**
     * set dropCaps Length in characters
     * @see setDropCapsLength
     */
    int dropCapsLength() const;
    /**
     * Set the number of lines that the drop-caps span
     * @see setDropCapsLength
     * @see setDropCaps
     * @see dropCapsDistance
     */
    void setDropCapsLines(int lines);
    /**
     * The dropCapsLines
     * @see setDropCapsLines
     */
    int dropCapsLines() const;
    /**
     * set the distance between drop caps and text in pt
     * @see setDropCapsLength
     * @see setDropCaps
     * @see setDropCapsLines
     */
    void setDropCapsDistance(qreal distance);
    /**
     * The dropCaps distance
     * @see setDropCapsDistance
     */
    qreal dropCapsDistance() const;

    /**
     * Set the style id of the text style used for dropcaps
     * @see setDropCapsDistance
     */
    void setDropCapsTextStyleId(int id);

    /**
     * The style id of the text style used for dropcaps
     * @see setDropCapsTextStyleId
     */
    int dropCapsTextStyleId() const;

    /**
     * If true the baselines will be aligned with the doc-wide grid
     */
    void setFollowDocBaseline(bool on);
    /**
     * return if baseline alignment is used
     * @see setFollowDocBaseline
     */
    bool followDocBaseline() const;

    /// See similar named method on QTextBlockFormat
    void setBackground(const QBrush &brush);
    /// See similar named method on QTextBlockFormat
    QBrush background() const;
    /// See similar named method on QTextBlockFormat
    void clearBackground();

    void setBreakBefore(bool on);
    bool breakBefore();
    void setBreakAfter(bool on);
    bool breakAfter();
    void setLeftPadding(qreal padding);
    qreal leftPadding();
    void setTopPadding(qreal padding);
    qreal topPadding();
    void setRightPadding(qreal padding);
    qreal rightPadding();
    void setBottomPadding(qreal padding);
    qreal bottomPadding();
    void setPadding(qreal padding);

    void setLeftBorderWidth(qreal width);
    qreal leftBorderWidth();
    void setLeftInnerBorderWidth(qreal width);
    qreal leftInnerBorderWidth();
    void setLeftBorderSpacing(qreal width);
    qreal leftBorderSpacing();
    void setLeftBorderStyle(KoBorder::BorderStyle style);
    KoBorder::BorderStyle leftBorderStyle();
    void setLeftBorderColor(const QColor &color);
    QColor leftBorderColor();
    void setTopBorderWidth(qreal width);
    qreal topBorderWidth();
    void setTopInnerBorderWidth(qreal width);
    qreal topInnerBorderWidth();
    void setTopBorderSpacing(qreal width);
    qreal topBorderSpacing();
    void setTopBorderStyle(KoBorder::BorderStyle style);
    KoBorder::BorderStyle topBorderStyle();
    void setTopBorderColor(const QColor &color);
    QColor topBorderColor();
    void setRightBorderWidth(qreal width);
    qreal rightBorderWidth();
    void setRightInnerBorderWidth(qreal width);
    qreal rightInnerBorderWidth();
    void setRightBorderSpacing(qreal width);
    qreal rightBorderSpacing();
    void setRightBorderStyle(KoBorder::BorderStyle style);
    KoBorder::BorderStyle rightBorderStyle();
    void setRightBorderColor(const QColor &color);
    QColor rightBorderColor();
    void setBottomBorderWidth(qreal width);
    qreal bottomBorderWidth();
    void setBottomInnerBorderWidth(qreal width);
    qreal bottomInnerBorderWidth();
    void setBottomBorderSpacing(qreal width);
    qreal bottomBorderSpacing();
    void setBottomBorderStyle(KoBorder::BorderStyle style);
    KoBorder::BorderStyle bottomBorderStyle();
    void setBottomBorderColor(const QColor &color);
    QColor bottomBorderColor();

    KoText::Direction textProgressionDirection() const;
    void setTextProgressionDirection(KoText::Direction dir);

    // ************ properties from QTextBlockFormat
    /// duplicated property from QTextBlockFormat
    void setTopMargin(qreal topMargin);
    /// duplicated property from QTextBlockFormat
    qreal topMargin() const;
    /// duplicated property from QTextBlockFormat
    void setBottomMargin(qreal margin);
    /// duplicated property from QTextBlockFormat
    qreal bottomMargin() const;
    /// duplicated property from QTextBlockFormat
    void setLeftMargin(qreal margin);
    /// duplicated property from QTextBlockFormat
    qreal leftMargin() const;
    /// duplicated property from QTextBlockFormat
    void setRightMargin(qreal margin);
    /// duplicated property from QTextBlockFormat
    qreal rightMargin() const;
    /// set the margin around the paragraph, making the margin on all sides equal.
    void setMargin(qreal margin);

    void setIsListHeader(bool on);
    bool isListHeader() const;

    /// duplicated property from QTextBlockFormat
    void setAlignment(Qt::Alignment alignment);
    /// duplicated property from QTextBlockFormat
    Qt::Alignment alignment() const;
    /// duplicated property from QTextBlockFormat
    void setTextIndent(qreal margin);
    /// duplicated property from QTextBlockFormat
    qreal textIndent() const;
    /// Custom KoParagraphStyle property for auto-text-indent
    void setAutoTextIndent(bool on);
    bool autoTextIndent() const;

#if 0
    as this is a duplicate of leftMargin, lets make it very clear we are using that one.
    /// duplicated property from QTextBlockFormat
    void setIndent(int indent);
    /// duplicated property from QTextBlockFormat
    int indent() const;
#endif
    /// duplicated property from QTextBlockFormat
    void setNonBreakableLines(bool on);
    /// duplicated property from QTextBlockFormat
    bool nonBreakableLines() const;

    /// set the parent style this one inherits its unset properties from.
    void setParentStyle(KoParagraphStyle *parent);

    /// return the parent style
    KoParagraphStyle *parentStyle() const;

    /// the 'next' style is the one used when the user creates a new paragrap after this one.
    void setNextStyle(int next);

    /// the 'next' style is the one used when the user creates a new paragrap after this one.
    int nextStyle() const;

    /// return the name of the style.
    QString name() const;

    /// set a user-visible name on the style.
    void setName(const QString &name);

    /// each style has a unique ID (non persistent) given out by the styleManager
    int styleId() const;

    /// each style has a unique ID (non persistent) given out by the styleManager
    void setStyleId(int id);

    /// return the optional name of the master-page or a QString() if this paragraph isn't attached to a master-page.
    QString masterPageName() const;
    /// Set the name of the master-page.
    void setMasterPageName(const QString &name);


    /// Set the list start value
    void setListStartValue(int value);
    /// Returns the list start value
    int listStartValue() const;

    /// Set to true if this paragraph is marked to start the list numbering from the first entry.
    void setRestartListNumbering(bool on);
    /// return if this paragraph is marked to start the list numbering from the first entry.
    bool restartListNumbering();

    /// Set the tab stop distance for this paragraph style.
    void setTabStopDistance(qreal value);
    /// return the tab stop distance for this paragraph style
    qreal tabStopDistance() const;
    /// Set the tab data for this paragraph style.
    void setTabPositions(const QList<KoText::Tab> &tabs);
    /// return the tabs data for this paragraph style
    QList<KoText::Tab> tabPositions() const;

    /// If this style is a list, then this sets the nested-ness (aka level) of this paragraph.  A H2 has level 2.
    void setListLevel(int value);
    /// return the list level.
    int listLevel() const;

    /**
     * Return the outline level of this block, or 0 if it's not a heading.
     * This information is here and not in the styles because the OpenDocument specification says so.
     * See ODF Spec 1.1, §14.1, Outline Numbering Level, but also other parts of the specification.
     */
    int outlineLevel() const;

    /**
     * Change this block outline level
     */
    void setOutlineLevel(int outline);

    /**
     * Return the default outline level of this style, or 0 if there is none.
     */
    int defaultOutlineLevel() const;

    /**
     * Change the default outline level for this style.
     */
    void setDefaultOutlineLevel(int outline);


    /**
     * 15.5.30: The text:number-lines attribute controls whether or not lines are numbered
     */
    bool lineNumbering() const;
    void setLineNumbering(bool lineNumbering);

    /**
     * 15.5.31:
     * The text:line-number property specifies a new start value for line numbering. The attribute is
     * only recognized if there is also a text:number-lines attribute with a value of true in the
     * same properties element.
     */
    int lineNumberStartValue() const;
    void setLineNumberStartValue(int lineNumberStartValue);


    /// copy all the properties from the other style to this style, effectively duplicating it.
    void copyProperties(const KoParagraphStyle *style);

    void unapplyStyle(QTextBlock &block) const;

    /**
     * Apply this style to a blockFormat by copying all properties from this, and parent
     * styles to the target block format.  Note that the character format will not be applied
     * using this method, use the other applyStyle() method for that.
     */
    void applyStyle(QTextBlockFormat &format) const;

    /**
     * Apply this style to the textBlock by copying all properties from this, parent and
     * the character style (where relevant) to the target block formats.
     */
    void applyStyle(QTextBlock &block, bool applyListStyle = true) const;

    /// return the character style for this paragraph style
    KoCharacterStyle *characterStyle();
    /// return the character style for this paragraph style
    const KoCharacterStyle *characterStyle() const;
    /// set the character style for this paragraph style
    void setCharacterStyle(KoCharacterStyle *style);

    /**
     * Returns the list style for this paragraph style.
     * @see KoListStyle::isValid()
     * @see setListStyle()
     * @see removeListStyle()
     */
    KoListStyle *listStyle() const;
    /**
     * Set a new liststyle on this paragraph style, making all paragraphs that use this style
     *  automatically be part of the list.
     * @see setListStyle()
     * @see removeListStyle()
     */
    void setListStyle(KoListStyle *style);

    void remove(int key);

    /// Compare the paragraph, character and list properties of this style with the other
    bool operator==(const KoParagraphStyle &other) const;
    /// Compare the paragraph properties of this style with other
    bool compareParagraphProperties(const KoParagraphStyle &other) const;
    /// Compare the character properties of this style with other
    bool compareCharacterProperties(const KoParagraphStyle &other) const;

    void removeDuplicates(const KoParagraphStyle &other);

    /**
     * Load the style form the element
     *
     * @param context the odf loading context
     * @param element the element containing the
     */
    void loadOdf(const KoXmlElement *element, KoShapeLoadingContext &context);

    void saveOdf(KoGenStyle &style, KoGenStyles &mainStyles);

    /**
     * Returns true if this paragraph style has the property set.
     * Note that this method does not delegate to the parent style.
     * @param key the key as found in the Property enum
     */
    bool hasProperty(int key) const;

    /**
     * Set a property with key to a certain value, overriding the value from the parent style.
     * If the value set is equal to the value of the parent style, the key will be removed instead.
     * @param key the Property to set.
     * @param value the new value to set on this style.
     * @see hasProperty(), value()
     */
    void setProperty(int key, const QVariant &value);
    /**
     * Return the value of key as represented on this style, taking into account parent styles.
     * You should consider using the direct accessors for individual properties instead.
     * @param key the Property to request.
     * @returns a QVariant which holds the property value.
     */
    QVariant value(int key) const;

signals:
    void nameChanged(const QString &newName);

private:
    /**
     * Load the style from the \a KoStyleStack style stack using the
     * OpenDocument format.
     */
    void loadOdfProperties(KoShapeLoadingContext &scontext);
    qreal propertyDouble(int key) const;
    int propertyInt(int key) const;
    bool propertyBoolean(int key) const;
    QColor propertyColor(int key) const;

    class Private;
    Private * const d;

    bool normalLineHeight;
};

#endif
