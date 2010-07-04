/* This file is part of the KDE project
 * Copyright (C) 2007,2010 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *   Contact: Suresh Chande suresh.chande@nokia.com
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

#ifndef KOENHANCEDPATHSHAPE_H
#define KOENHANCEDPATHSHAPE_H

#include <KoParameterShape.h>
#include <QList>
#include <QMap>
#include <QRectF>

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
    explicit EnhancedPathShape(const QRectF &viewBox);
    virtual ~EnhancedPathShape();

    /**
     * Evaluates the given reference to a identifier, modifier or formula.
     * @param reference the reference to evaluate
     * @return the result of the evaluation
     */
    qreal evaluateReference(const QString &reference);

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
    virtual void setSize(const QSizeF &newSize);
    // from KoParameterShape
    virtual QPointF normalize();

    /// Add formula with given name and textual represenation
    void addFormula(const QString &name, const QString &formula);

    /// Add a single handle with format: x y minX maxX minY maxY
    void addHandle(const QMap<QString,QVariant> &handle);

    /// Add modifiers with format: modifier0 modifier1 modifier2 ...
    void addModifiers(const QString &modifiers);

    /// Add command for instance "M 0 0"
    void addCommand(const QString &command);

    /// Returns the viewbox of the enhanced path shape
    QRectF viewBox() const;

    /// Converts from shape coordinates to viewbox coordinates
    QPointF shapeToViewbox(const QPointF &point) const;

    /// Sets if the shape is to be mirrored horizontally before aplying any other transformations
    //NOTE: in the standard nothing is mentioned about the priorities of the transformations"
    //it's assumed like this because of the behavior shwon in OOo
    void setMirrorHorizontally(bool mirrorHorizontally);

    /// Sets if the shape is to be mirrored vertically before aplying any other transformations
    //NOTE: in the standard nothing is mentioned about the priorities of the transformations"
    //it's assumed like this because of the behavior shwon in OOo
    void setMirrorVertically(bool mirrorVertically);

    /// Returns parameter from given textual representation
    EnhancedPathParameter *parameter(const QString &text);

protected:
    // from KoShape
    virtual void saveOdf(KoShapeSavingContext &context) const;
    // from KoShape
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);
    // from KoParameterShape
    virtual void moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    // from KoParameterShape
    virtual void updatePath(const QSizeF &size);
private:

    void evaluateHandles();
    void reset();

    /// parses the enhanced path data
    void parsePathData(const QString &data);

    /// Adds a new command
    void addCommand(const QString &command, bool triggerUpdate);

    typedef QMap<QString, EnhancedPathFormula*> FormulaStore;
    typedef QList<qreal> ModifierStore;
    typedef QMap<QString, EnhancedPathParameter*> ParameterStore;

    QRectF m_viewBox;     ///< the viewbox rectangle
    QRectF m_viewBound;   ///< the bounding box of the path in viewbox coordinates
    QTransform m_viewMatrix; ///< matrix to convert from viewbox coordinates to shape coordinates
    QPointF m_viewBoxOffset;
    QList<EnhancedPathCommand*> m_commands; ///< the commands creating the outline
    QList<EnhancedPathHandle*> m_enhancedHandles; ///< the handles for modifiying the shape
    FormulaStore m_formulae;     ///< the formulae
    ModifierStore m_modifiers;   ///< the modifier values
    ParameterStore m_parameters; ///< the shared parameters
    bool m_mirrorVertically; ///<whether or not the shape is to be mirrored vertically before transforming it
    bool m_mirrorHorizontally; ///<whether or not the shape is to be mirrored horizontally before transforming it
};

#endif // KOENHANCEDPATHSHAPE_H
