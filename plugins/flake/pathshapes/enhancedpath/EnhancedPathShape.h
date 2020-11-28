/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007, 2010, 2011 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2010 Carlos Licea <carlos@kdab.com>
 * SPDX-FileCopyrightText: 2010 Nokia Corporation and /or its subsidiary(-ies).
 *   Contact: Suresh Chande suresh.chande@nokia.com
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOENHANCEDPATHSHAPE_H
#define KOENHANCEDPATHSHAPE_H

#include <KoParameterShape.h>
#include <QList>
#include <QMap>
#include <QRectF>
#include <QStringList>

#define EnhancedPathShapeId "EnhancedPathShape"

class EnhancedPathCommand;
class EnhancedPathHandle;
class EnhancedPathFormula;
class EnhancedPathParameter;
class KoShapeSavingContext;
class KoShapeLoadingContext;

/**
 * An enhanced shape is a custom shape which can be defined
 * by enhanced geometry data.
 * The data consists of a list of commands like moveto,
 * lineto, curveto, etc. which are used to create the outline
 * of the shape. The coordinates or parameters of the commands
 * can be constant values, named variables (identifiers),
 * modifiers, functions or formulae.
*/
class EnhancedPathShape : public KoParameterShape
{
public:
    EnhancedPathShape(const QRect &viewBox);
    ~EnhancedPathShape() override;

    KoShape* cloneShape() const override;

    /**
     * Evaluates the given reference to a identifier, modifier or formula.
     * @param reference the reference to evaluate
     * @return the result of the evaluation
     */
    qreal evaluateReference(const QString &reference);

    /**
     * Evaluates the given constant or reference to a identifier, modifier
     * or formula.
     * @param val the value to evaluate
     * @return the result of the evaluation
     */
    qreal evaluateConstantOrReference(const QString &val);

    /**
     * Attempts to modify a given reference.
     *
     * Only modifiers can me modified, others silently ignore the attempt.
     *
     * @param reference the reference to modify
     * @param value the new value
     */
    void modifyReference(const QString &reference, qreal value);

    // from KoShape
    void setSize(const QSizeF &newSize) override;
    // from KoParameterShape
    QPointF normalize() override;

    /// Add formula with given name and textual representation
    void addFormula(const QString &name, const QString &formula);

    /// Add a single handle with format: x y minX maxX minY maxY
    void addHandle(const QMap<QString, QVariant> &handle);

    /// Add modifiers with format: modifier0 modifier1 modifier2 ...
    void addModifiers(const QString &modifiers);

    /// Add command for instance "M 0 0"
    void addCommand(const QString &command);

    /// Returns the viewbox of the enhanced path shape
    QRect viewBox() const;

    /// Converts from shape coordinates to viewbox coordinates
    QPointF shapeToViewbox(const QPointF &point) const;

    /// Sets if the shape is to be mirrored horizontally before applying any other transformations
    //NOTE: in the standard nothing is mentioned about the priorities of the transformations"
    //it's assumed like this because of the behavior shwon in OOo
    void setMirrorHorizontally(bool mirrorHorizontally);

    /// Sets if the shape is to be mirrored vertically before applying any other transformations
    //NOTE: in the standard nothing is mentioned about the priorities of the transformations"
    //it's assumed like this because of the behavior shown in OOo
    void setMirrorVertically(bool mirrorVertically);

    // Sets member variable representing draw:path-stretchpoint-x attribute
    void setPathStretchPointX(qreal pathStretchPointX);

    // Sets member variable representing draw:path-stretchpoint-y attribute
    void setPathStretchPointY(qreal pathStretchPointY);

    /// Returns parameter from given textual representation
    EnhancedPathParameter *parameter(const QString &text);

protected:
    //from KoShape
    void shapeChanged(ChangeType type, KoShape *shape = 0) override;
    // from KoParameterShape
    void moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers = Qt::NoModifier) override;
    // from KoParameterShape
    void updatePath(const QSizeF &size) override;

private:
    EnhancedPathShape(const EnhancedPathShape &rhs);

    void evaluateHandles();
    void reset();

    /// parses the enhanced path data
    void parsePathData(const QString &data);

    /// Adds a new command
    void addCommand(const QString &command, bool triggerUpdate);

    /// Updates the size and position of an optionally existing text-on-shape text area
    void updateTextArea();

    /// Enables caching results
    void enableResultCache(bool enable);

    // This function checks if draw:path-stretchpoint-x or draw:path-stretchpoint-y attributes are set.
    // If the attributes are set the path shape coordinates (m_subpaths) are changed so that the form
    // of the shape is preserved after stretching. It is needed for example in round-rectangles, to
    // have the corners round after stretching. Without it the corners would be elliptical.
    // Returns true if any points were actually changed, otherwise false.
    bool useStretchPoints(const QSizeF &size, qreal &scale);

    typedef QMap<QString, EnhancedPathFormula *> FormulaStore;
    typedef QList<qreal> ModifierStore;
    typedef QMap<QString, EnhancedPathParameter *> ParameterStore;

    QRect m_viewBox;     ///< the viewbox rectangle
    QRectF m_viewBound;   ///< the bounding box of the path in viewbox coordinates
    QTransform m_viewMatrix; ///< matrix to convert from viewbox coordinates to shape coordinates
    QTransform m_mirrorMatrix; ///< matrix to used for mirroring
    QPointF m_viewBoxOffset;
    QStringList m_textArea;
    QList<EnhancedPathCommand *> m_commands; ///< the commands creating the outline
    QList<EnhancedPathHandle *> m_enhancedHandles; ///< the handles for modifying the shape
    FormulaStore m_formulae;     ///< the formulae
    ModifierStore m_modifiers;   ///< the modifier values
    ParameterStore m_parameters; ///< the shared parameters
    bool m_mirrorVertically; ///<whether or not the shape is to be mirrored vertically before transforming it
    bool m_mirrorHorizontally; ///<whether or not the shape is to be mirrored horizontally before transforming it
    qreal m_pathStretchPointX; ///< draw:path-stretchpoint-x attribute
    qreal m_pathStretchPointY; ///< draw:path-stretchpoint-y attribute
    QHash<QString, qreal> m_resultCache; ///< cache for intermediate results used when evaluating path
    bool m_cacheResults; ///< indicates if result cache is enabled
};

#endif // KOENHANCEDPATHSHAPE_H
