/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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
    ~EnhancedPathShape();

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
    const QRectF & viewBox() const;
    QPointF shapeToViewbox(const QPointF &point) const;
    QPointF viewboxToShape(const QPointF &point) const;
    qreal shapeToViewbox(qreal value) const;
    qreal viewboxToShape(qreal value) const;

    /// Returns parameter from given textual representation
    EnhancedPathParameter *parameter(const QString &text);

protected:
    void saveOdf(KoShapeSavingContext &context) const;
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);
    // from KoParameterShape
    void moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    // from KoParameterShape
    void updatePath(const QSizeF &size);
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

    QRectF m_viewBox;
    QMatrix m_viewMatrix;
    QPointF m_viewBoxOffset;
    QList<EnhancedPathCommand*> m_commands; ///< the commands creating the outline
    QList<EnhancedPathHandle*> m_enhancedHandles; ///< the handles for modifiying the shape
    FormulaStore m_formulae;     ///< the formulae
    ModifierStore m_modifiers;   ///< the modifier values
    ParameterStore m_parameters; ///< the shared parameters
};

#endif // KOENHANCEDPATHSHAPE_H
