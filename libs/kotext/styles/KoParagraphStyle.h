/* This file is part of the KDE project
 * Copyright (C) 2006, 2007 Thomas Zander <zander@kde.org>
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
#ifndef KOPARAGRAPHSTYLE_H
#define KOPARAGRAPHSTYLE_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QVariant>
#include <QTextFormat>
#include <kotext_export.h>

struct Property;
class KoCharacterStyle;
class KoListStyle;
class StylePrivate;
class QTextBlock;
class KoStyleStack;

/**
 * A container for all properties for the paragraph wide style.
 * Each paragraph in the main text either is based on a parag style, or its not. Where
 * it is based on a paragraph style this is indecated that it has a property 'StyleId'
 * with an integer as value.  The integer value corresponds to the styleId() output of
 * a specific KoParagraphStyle.
 * @see KoStyleManager
 */
class KOTEXT_EXPORT KoParagraphStyle : public QObject {
    Q_OBJECT
public:
    enum Property {
        StyleId = QTextFormat::UserProperty+1,
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
        FollowDocBaseline,  ///< If true the baselines will be aligned with the doc-wide grid
        BreakBefore,    ///< If true, insert a frame break before this paragraph
        BreakAfter,     ///< If true, insert a frame break after this paragraph

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
        ExplicitListValue, ///< Int with the list-value that that parag will have. Ignored if this is not a list.
        RestartListNumbering    ///< boolean to indicate that this paragraph will have numbering restart at the list-start. Ignored if this is not a list.

// do 15.5.24
// continue at 15.5.28
    };

    enum BorderStyle {
        BorderNone,   ///< no border. This value forces the computed value of 'border-width' to be '0'.
        BorderDotted,   ///< The border is a series of dots.
        BorderDashed,   ///< The border is a series of short line segments.
        BorderSolid,    ///< The border is a single line segment.
        BorderDouble,   ///< The border is two solid lines. The sum of the two lines and the space between them equals the value of 'border-width'.
        BorderGroove,   ///< The border looks as though it were carved into the canvas.
        BorderRidge,    ///< The opposite of 'groove': the border looks as though it were coming out of the canvas.
        BorderInset,    ///< The border makes the entire box look as though it were embedded in the canvas.
        BorderOutset,   ///< The opposite of 'inset': the border makes the entire box look as though it were coming out of the canvas.

        // kword legacy
        BorderDashDotPattern,
        BorderDashDotDotPattern
    };


    KoParagraphStyle();
    /// Copy constructor
    KoParagraphStyle(const KoParagraphStyle &orig);
    ~KoParagraphStyle();

    //  ***** Linespacing
    /**
     * Sets the line height as a percentage of the highest character on that line.
     * A good typographically correct value would be 120%
     * Note that lineSpacing() is added to this.
     * @see setLineSpacingFromFont
     */
    void setLineHeightPercent(int lineHeight)
       ;
    int lineHeightPercent() const;

    /**
     * Sets the line height to a specific pt-based height, ignoring the font size.
     * Note that lineSpacing() is added to this.
     */
    void setLineHeightAbsolute(double height)
       ;
    double lineHeightAbsolute() const;

    /**
     * Sets the line height to have a minimum height in pt.
     */
    void setMinimumLineHeight(double height);
    double minimumLineHeight() const;

    /**
     * Sets the space between two lines to be a specific height. The total linespacing will become
     * the line height + this height.  Where the line height is dependent on the font.
     */
    void setLineSpacing(double spacing)
       ;
    double lineSpacing() const;

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
    void setAlignLastLine(Qt::Alignment alignment );
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
     * set dropCapsLines
     * @see setDropCapsLines
     */
    int dropCapsLines() const;
    /**
     * set the distance between drop caps and text in pt
     * @see setDropCapsLength
     * @see setDropCaps
     * @see setDropCapsLines
     */
    void setDropCapsDistance(double distance);
    /**
     * Set dropCaps distance
     * @see setDropCapsDistance
     */
    double dropCapsDistance() const;
    /**
     * If true the baselines will be aligned with the doc-wide grid
     */
    void setFollowDocBaseline(bool on);
    /**
     * return if baseline alignment is used
     * @see setFollowDocBaseline
     */
    bool followDocBaseline() const;

    void setBreakBefore(bool on);
    bool breakBefore();
    void setBreakAfter(bool on);
    bool breakAfter();
    void setLeftPadding(double padding);
    double leftPadding();
    void setTopPadding(double padding);
    double topPadding();
    void setRightPadding(double padding);
    double rightPadding();
    void setBottomPadding(double padding);
    double bottomPadding();

    void setLeftBorderWidth(double width);
    double leftBorderWidth();
    void setLeftInnerBorderWidth(double width);
    double leftInnerBorderWidth();
    void setLeftBorderSpacing(double width);
    double leftBorderSpacing();
    void setLeftBorderStyle(BorderStyle style);
    BorderStyle leftBorderStyle();
    void setLeftBorderColor(QColor color);
    QColor leftBorderColor();
    void setTopBorderWidth(double width);
    double topBorderWidth();
    void setTopInnerBorderWidth(double width);
    double topInnerBorderWidth();
    void setTopBorderSpacing(double width);
    double topBorderSpacing();
    void setTopBorderStyle(BorderStyle style);
    BorderStyle topBorderStyle();
    void setTopBorderColor(QColor color);
    QColor topBorderColor();
    void setRightBorderWidth(double width);
    double rightBorderWidth();
    void setRightInnerBorderWidth(double width);
    double rightInnerBorderWidth();
    void setRightBorderSpacing(double width);
    double rightBorderSpacing();
    void setRightBorderStyle(BorderStyle style);
    BorderStyle rightBorderStyle();
    void setRightBorderColor(QColor color);
    QColor rightBorderColor();
    void setBottomBorderWidth(double width);
    double bottomBorderWidth();
    void setBottomInnerBorderWidth(double width);
    double bottomInnerBorderWidth();
    void setBottomBorderSpacing(double width);
    double bottomBorderSpacing();
    void setBottomBorderStyle(BorderStyle style);
    BorderStyle bottomBorderStyle();
    void setBottomBorderColor(QColor color);
    QColor bottomBorderColor();


    // ************ properties from QTextFormat
    /// duplicated property from QTextBlockFormat
    void setTopMargin(double topMargin);
    /// duplicated property from QTextBlockFormat
    double topMargin() const;

    /// duplicated property from QTextBlockFormat
    void setBottomMargin (double margin);
    /// duplicated property from QTextBlockFormat
    double bottomMargin () const;
    /// duplicated property from QTextBlockFormat
    void setLeftMargin (double margin);
    /// duplicated property from QTextBlockFormat
    double leftMargin () const;
    /// duplicated property from QTextBlockFormat
    void setRightMargin (double margin);
    /// duplicated property from QTextBlockFormat
    double rightMargin () const;

    /// duplicated property from QTextBlockFormat
    void setAlignment (Qt::Alignment alignment);
    /// duplicated property from QTextBlockFormat
    Qt::Alignment alignment () const;
    /// duplicated property from QTextBlockFormat
    void setTextIndent (double margin);
    /// duplicated property from QTextBlockFormat
    double textIndent () const;
#if 0
as this is a duplicate of leftMargin, lets make it very clear we are using that one.
    /// duplicated property from QTextBlockFormat
    void setIndent (int indent);
    /// duplicated property from QTextBlockFormat
    int indent () const;
#endif
    /// duplicated property from QTextBlockFormat
    void setNonBreakableLines(bool on);
    /// duplicated property from QTextBlockFormat
    bool nonBreakableLines() const;

    /// set the parent style this one inherits its unset properties from.
    void setParent(KoParagraphStyle *parent);

    /// return the parent style
    KoParagraphStyle *parent() const;

    /// the 'next' style is the one used when the user creates a new paragrap after this one.
    void setNextStyle(int next);

    /// the 'next' style is the one used when the user creates a new paragrap after this one.
    int nextStyle() const;

    /// return the name of the style.
    const QString& name() const;

    /// set a user-visible name on the style.
    void setName(const QString &name);

    /// each style has a unique ID (non persistent) given out by the styleManager
    int styleId() const;

    /// each style has a unique ID (non persistent) given out by the styleManager
    void setStyleId(int id);

    /// Set to true if this paragraph is marked to start the list numbering from the first entry.
    void setRestartListNumbering(bool on);
    /// return if this paragraph is marked to start the list numbering from the first entry.
    bool restartListNumbering();

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
    void applyStyle(QTextBlock &block) const;

    KoCharacterStyle *characterStyle();
    const KoCharacterStyle *characterStyle() const;

    /**
     * Returns the list style for this paragraph style, or 0 if there is none.
     * @see setListStyle()
     * @see removeListStyle()
     */
    KoListStyle *listStyle();
    /**
     * Returns the list style for this paragraph style, or 0 if there is none.
     * @see setListStyle()
     * @see removeListStyle()
     */
    const KoListStyle *listStyle() const;

    /**
     * Set a new liststyle on this paragraph style, making all paragraphs that use this style
     *  automatically be part of the list.
     * @see setListStyle()
     * @see removeListStyle()
     */
    void setListStyle(const KoListStyle &style);
    /**
     * Remove any set list style on this paragraphs style.
     * Stops all paragraphs that follow this style from being a list item.
     */
    void removeListStyle();

    /**
     * Load the style from the \a KoStyleStack style stack using the
     * OpenDocument format.
     */
    void loadOasis(KoStyleStack& styleStack);

    static KoParagraphStyle *fromBlockFormat(const QTextBlockFormat &format);

private:
    void setProperty(int key, const QVariant &value);
    void remove(int key);
    double propertyDouble(int key) const;
    int propertyInt(int key) const;
    bool propertyBoolean(int key) const;
    QColor propertyColor(int key) const;
    QVariant const *get(int key) const;

    class Private;
    Private * const d;
};

#endif
