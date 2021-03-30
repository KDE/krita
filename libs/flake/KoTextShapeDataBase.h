/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOTEXTSHAPEDATABASE_H
#define KOTEXTSHAPEDATABASE_H

#include "kritaflake_export.h"

#include "KoShapeUserData.h"

class KoTextShapeDataBasePrivate;
#include <QDomDocument>
class KoShapeLoadingContext;
class KoShapeSavingContext;
struct KoInsets;

class QTextDocument;

/**
 * \internal
 */
class KRITAFLAKE_EXPORT KoTextShapeDataBase : public KoShapeUserData
{
    Q_OBJECT
public:
    /// constructor
    KoTextShapeDataBase();
    ~KoTextShapeDataBase() override;

    /// return the document
    QTextDocument *document() const;

    /**
     * Set the margins that will make the shapes text area smaller.
     * The shape that owns this textShapeData object will layout text in an area
     * confined by the shape size made smaller by the margins set here.
     * @param margins the margins that shrink the text area.
     */
    void setShapeMargins(const KoInsets &margins);
    /**
     * returns the currently set margins for the shape.
     */
    KoInsets shapeMargins() const;

    /** Sets the vertical alignment of all the text inside the shape. */
    void setVerticalAlignment(Qt::Alignment alignment);

    /** Returns the vertical alignment of all the text in the shape */
    Qt::Alignment verticalAlignment() const;

    /**
     * Enum to describe the text document's automatic resizing behaviour.
     */
    enum ResizeMethod {
        /// Resize the shape to fit the content. This makes sure that the text shape takes op
        /// only as much space as absolutely necessary to fit the entire text into its boundaries.
        AutoResize,
        /// Specifies whether or not to automatically increase the width of the drawing object
        /// if text is added to fit the entire width of the text into its boundaries.
        /// Compared to AutoResize above this only applied to the width whereas the height is
        /// not resized. Also this only grows but does not shrink again if text is removed again.
        AutoGrowWidth,
        /// Specifies whether or not to automatically increase the height of the drawing object
        /// if text is added to fit the entire height of the text into its boundaries.
        AutoGrowHeight,
        /// This combines the AutoGrowWidth and AutoGrowHeight and automatically increase width
        /// and height to fit the entire text into its boundaries.
        AutoGrowWidthAndHeight,
        /// Shrink the content displayed within the shape to match into the shape's boundaries. This
        /// will scale the content down as needed to display the whole document.
        ShrinkToFitResize,
        /// Deactivates auto-resizing. This is the default resizing method.
        NoResize
    };

    /**
     * Specifies how the document should be resized upon a change in the document.
     *
     * If auto-resizing is turned on, text will not be wrapped unless enforced by e.g. a newline.
     *
     * By default, NoResize is set.
     */
    void setResizeMethod(ResizeMethod method);

    /**
     * Returns the auto-resizing mode. By default, this is NoResize.
     *
     * @see setResizeMethod
     */
    ResizeMethod resizeMethod() const;

protected:
    /// constructor
    KoTextShapeDataBase(KoTextShapeDataBasePrivate *);

    KoTextShapeDataBasePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(KoTextShapeDataBase)
};

#endif

