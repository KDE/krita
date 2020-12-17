/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007, 2010, 2011 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2009-2010 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2010 Carlos Licea <carlos@kdab.com>
 * SPDX-FileCopyrightText: 2010 Nokia Corporation and /or its subsidiary(-ies).
 *   Contact: Suresh Chande suresh.chande@nokia.com
 * SPDX-FileCopyrightText: 2009-2010 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <KoParameterShape_p.h>

#include "EnhancedPathShape.h"
#include "EnhancedPathCommand.h"
#include "EnhancedPathParameter.h"
#include "EnhancedPathHandle.h"
#include "EnhancedPathFormula.h"

#include <QPainterPath>

#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoShapeSavingContext.h>
#include <KoUnit.h>
#include <KoPathPoint.h>

EnhancedPathShape::EnhancedPathShape(const QRect &viewBox)
    : m_viewBox(viewBox)
    , m_viewBoxOffset(0.0, 0.0)
    , m_mirrorVertically(false)
    , m_mirrorHorizontally(false)
    , m_pathStretchPointX(-1)
    , m_pathStretchPointY(-1)
    , m_cacheResults(false)
{
}

EnhancedPathShape::EnhancedPathShape(const EnhancedPathShape &rhs)
    : KoParameterShape(rhs),
      m_viewBox(rhs.m_viewBox),
      m_viewBound(rhs.m_viewBound),
      m_viewMatrix(rhs.m_viewMatrix),
      m_mirrorMatrix(rhs.m_mirrorMatrix),
      m_viewBoxOffset(rhs.m_viewBoxOffset),
      m_textArea(rhs.m_textArea),
      m_commands(rhs.m_commands),
      m_enhancedHandles(rhs.m_enhancedHandles),
      m_formulae(rhs.m_formulae),
      m_modifiers(rhs.m_modifiers),
      m_parameters(rhs.m_parameters),
      m_mirrorVertically(rhs.m_mirrorVertically),
      m_mirrorHorizontally(rhs.m_mirrorHorizontally),
      m_pathStretchPointX(rhs.m_pathStretchPointX),
      m_pathStretchPointY(rhs.m_pathStretchPointY),
      m_resultCache(rhs.m_resultCache),
      m_cacheResults(rhs.m_cacheResults)
{
}

EnhancedPathShape::~EnhancedPathShape()
{
    reset();
}

KoShape *EnhancedPathShape::cloneShape() const
{
    return new EnhancedPathShape(*this);
}

void EnhancedPathShape::reset()
{
    qDeleteAll(m_commands);
    m_commands.clear();
    qDeleteAll(m_enhancedHandles);
    m_enhancedHandles.clear();
    setHandles(QList<QPointF>());
    qDeleteAll(m_formulae);
    m_formulae.clear();
    qDeleteAll(m_parameters);
    m_parameters.clear();
    m_modifiers.clear();
    m_viewMatrix.reset();
    m_viewBoxOffset = QPointF();
    clear();
    m_textArea.clear();
}

void EnhancedPathShape::moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    EnhancedPathHandle *handle = m_enhancedHandles[ handleId ];
    if (handle) {
        handle->changePosition(shapeToViewbox(point));
    }
}

void EnhancedPathShape::updatePath(const QSizeF &size)
{
    if (isParametricShape()) {
        clear();
        enableResultCache(true);

        foreach (EnhancedPathCommand *cmd, m_commands) {
            cmd->execute();
        }

        enableResultCache(false);

        qreal stretchPointsScale = 1;
        bool isStretched = useStretchPoints(size, stretchPointsScale);
        m_viewBound = outline().boundingRect();
        m_mirrorMatrix.reset();
        m_mirrorMatrix.translate(m_viewBound.center().x(), m_viewBound.center().y());
        m_mirrorMatrix.scale(m_mirrorHorizontally ? -1 : 1, m_mirrorVertically ? -1 : 1);
        m_mirrorMatrix.translate(-m_viewBound.center().x(), -m_viewBound.center().y());
        QTransform matrix(1.0, 0.0, 0.0, 1.0, m_viewBoxOffset.x(), m_viewBoxOffset.y());

        // if stretch points are set than stretch the path manually
        if (isStretched) {
            //if the path was stretched manually the stretch matrix is not more valid
            //and it has to be recalculated so that stretching in x and y direction is the same
            matrix.scale(stretchPointsScale, stretchPointsScale);
            matrix = m_mirrorMatrix * matrix;
        } else {
            matrix = m_mirrorMatrix * m_viewMatrix * matrix;
        }
        foreach (KoSubpath *subpath, subpaths()) {
            foreach (KoPathPoint *point, *subpath) {
                point->map(matrix);
            }
        }

        const int handleCount = m_enhancedHandles.count();
        QList<QPointF> handles;
        for (int i = 0; i < handleCount; ++i) {
            handles.append(matrix.map(m_enhancedHandles[i]->position()));
        }
        setHandles(handles);

        normalize();
    }
}

void EnhancedPathShape::setSize(const QSizeF &newSize)
{
    // handle offset
    KoParameterShape::setSize(newSize);

    // calculate scaling factors from viewbox size to shape size
    qreal xScale = m_viewBound.width() == 0 ? 1 : newSize.width() / m_viewBound.width();
    qreal yScale = m_viewBound.height() == 0 ? 1 : newSize.height() / m_viewBound.height();

    // create view matrix, take mirroring into account
    m_viewMatrix.reset();
    m_viewMatrix.scale(xScale, yScale);

    updatePath(newSize);
}

QPointF EnhancedPathShape::normalize()
{
    QPointF offset = KoParameterShape::normalize();

    m_viewBoxOffset -= offset;

    return offset;
}

QPointF EnhancedPathShape::shapeToViewbox(const QPointF &point) const
{
    return (m_mirrorMatrix * m_viewMatrix).inverted().map(point - m_viewBoxOffset);
}

void EnhancedPathShape::evaluateHandles()
{
    const int handleCount = m_enhancedHandles.count();
    QList<QPointF> handles;
    for (int i = 0; i < handleCount; ++i) {
        handles.append(m_enhancedHandles[i]->position());
    }
    setHandles(handles);
}

QRect EnhancedPathShape::viewBox() const
{
    return m_viewBox;
}

qreal EnhancedPathShape::evaluateReference(const QString &reference)
{
    if (reference.isEmpty()) {
        return 0.0;
    }

    const char c = reference[0].toLatin1();

    qreal res = 0.0;

    switch (c) {
    // referenced modifier
    case '$': {
        bool success = false;
        int modifierIndex = reference.mid(1).toInt(&success);
        res = m_modifiers.value(modifierIndex);
        break;
    }
    // referenced formula
    case '?': {
        QString fname = reference.mid(1);
        if (m_cacheResults && m_resultCache.contains(fname)) {
            res = m_resultCache.value(fname);
        } else {
            FormulaStore::const_iterator formulaIt = m_formulae.constFind(fname);
            if (formulaIt != m_formulae.constEnd()) {
                EnhancedPathFormula *formula = formulaIt.value();
                if (formula) {
                    res = formula->evaluate();
                    if (m_cacheResults) {
                        m_resultCache.insert(fname, res);
                    }
                }
            }
        }
        break;
    }
    // maybe an identifier ?
    default:
        EnhancedPathNamedParameter p(reference, this);
        res = p.evaluate();
        break;
    }

    return res;
}

qreal EnhancedPathShape::evaluateConstantOrReference(const QString &val)
{
    bool ok = true;
    qreal res = val.toDouble(&ok);
    if (ok) {
        return res;
    }
    return evaluateReference(val);
}

void EnhancedPathShape::modifyReference(const QString &reference, qreal value)
{
    if (reference.isEmpty()) {
        return;
    }

    const char c = reference[0].toLatin1();

    if (c == '$') {
        bool success = false;
        int modifierIndex = reference.mid(1).toInt(&success);
        if (modifierIndex >= 0 && modifierIndex < m_modifiers.count()) {
            m_modifiers[modifierIndex] = value;
        }
    }
}

EnhancedPathParameter *EnhancedPathShape::parameter(const QString &text)
{
    Q_ASSERT(! text.isEmpty());

    ParameterStore::const_iterator parameterIt = m_parameters.constFind(text);
    if (parameterIt != m_parameters.constEnd()) {
        return parameterIt.value();
    } else {
        EnhancedPathParameter *parameter = 0;
        const char c = text[0].toLatin1();
        if (c == '$' || c == '?') {
            parameter = new EnhancedPathReferenceParameter(text, this);
        } else {
            bool success = false;
            qreal constant = text.toDouble(&success);
            if (success) {
                parameter = new EnhancedPathConstantParameter(constant, this);
            } else {
                Identifier identifier = EnhancedPathNamedParameter::identifierFromString(text);
                if (identifier != IdentifierUnknown) {
                    parameter = new EnhancedPathNamedParameter(identifier, this);
                }
            }
        }

        if (parameter) {
            m_parameters[text] = parameter;
        }

        return parameter;
    }
}

void EnhancedPathShape::addFormula(const QString &name, const QString &formula)
{
    if (name.isEmpty() || formula.isEmpty()) {
        return;
    }

    m_formulae[name] = new EnhancedPathFormula(formula, this);
}

void EnhancedPathShape::addHandle(const QMap<QString, QVariant> &handle)
{
    if (handle.isEmpty()) {
        return;
    }

    if (!handle.contains("draw:handle-position")) {
        return;
    }
    QVariant position = handle.value("draw:handle-position");

    QStringList tokens = position.toString().simplified().split(' ');
    if (tokens.count() < 2) {
        return;
    }

    EnhancedPathHandle *newHandle = new EnhancedPathHandle(this);
    newHandle->setPosition(parameter(tokens[0]), parameter(tokens[1]));

    // check if we have a polar handle
    if (handle.contains("draw:handle-polar")) {
        QVariant polar = handle.value("draw:handle-polar");
        QStringList tokens = polar.toString().simplified().split(' ');
        if (tokens.count() == 2) {
            newHandle->setPolarCenter(parameter(tokens[0]), parameter(tokens[1]));

            QVariant minRadius = handle.value("draw:handle-radius-range-minimum");
            QVariant maxRadius = handle.value("draw:handle-radius-range-maximum");
            if (minRadius.isValid() && maxRadius.isValid()) {
                newHandle->setRadiusRange(parameter(minRadius.toString()), parameter(maxRadius.toString()));
            }
        }
    } else {
        QVariant minX = handle.value("draw:handle-range-x-minimum");
        QVariant maxX = handle.value("draw:handle-range-x-maximum");
        if (minX.isValid() && maxX.isValid()) {
            newHandle->setRangeX(parameter(minX.toString()), parameter(maxX.toString()));
        }

        QVariant minY = handle.value("draw:handle-range-y-minimum");
        QVariant maxY = handle.value("draw:handle-range-y-maximum");
        if (minY.isValid() && maxY.isValid()) {
            newHandle->setRangeY(parameter(minY.toString()), parameter(maxY.toString()));
        }
    }

    m_enhancedHandles.append(newHandle);

    evaluateHandles();
}

void EnhancedPathShape::addModifiers(const QString &modifiers)
{
    if (modifiers.isEmpty()) {
        return;
    }

    QStringList tokens = modifiers.simplified().split(' ');
    int tokenCount = tokens.count();
    for (int i = 0; i < tokenCount; ++i) {
        m_modifiers.append(tokens[i].toDouble());
    }
}

void EnhancedPathShape::addCommand(const QString &command)
{
    addCommand(command, true);
}

void EnhancedPathShape::addCommand(const QString &command, bool triggerUpdate)
{
    QString commandStr = command.simplified();
    if (commandStr.isEmpty()) {
        return;
    }

    // the first character is the command
    EnhancedPathCommand *cmd = new EnhancedPathCommand(commandStr[0], this);

    // strip command char
    commandStr = commandStr.mid(1).simplified();

    // now parse the command parameters
    if (!commandStr.isEmpty()) {
        QStringList tokens = commandStr.split(' ');
        for (int i = 0; i < tokens.count(); ++i) {
            cmd->addParameter(parameter(tokens[i]));
        }
    }
    m_commands.append(cmd);

    if (triggerUpdate) {
        updatePath(size());
    }
}

bool EnhancedPathShape::useStretchPoints(const QSizeF &size, qreal &scale)
{
    bool retval = false;
    if (m_pathStretchPointX != -1 && m_pathStretchPointY != -1) {
        qreal scaleX = size.width();
        qreal scaleY = size.height();
        if (qreal(m_viewBox.width()) / m_viewBox.height() < qreal(scaleX) / scaleY) {
            qreal deltaX = (scaleX * m_viewBox.height()) / scaleY - m_viewBox.width();
            foreach (KoSubpath *subpath, subpaths()) {
                foreach (KoPathPoint *currPoint, *subpath) {
                    if (currPoint->point().x() >=  m_pathStretchPointX &&
                            currPoint->controlPoint1().x() >= m_pathStretchPointX &&
                            currPoint->controlPoint2().x() >= m_pathStretchPointX) {
                        currPoint->setPoint(QPointF(currPoint->point().x() + deltaX, currPoint->point().y()));
                        currPoint->setControlPoint1(QPointF(currPoint->controlPoint1().x() + deltaX,
                                                            currPoint->controlPoint1().y()));
                        currPoint->setControlPoint2(QPointF(currPoint->controlPoint2().x() + deltaX,
                                                            currPoint->controlPoint2().y()));
                        retval = true;
                    }
                }
            }
            scale = scaleY / m_viewBox.height();
        } else if (qreal(m_viewBox.width()) / m_viewBox.height() > qreal(scaleX) / scaleY) {
            qreal deltaY = (m_viewBox.width() * scaleY) / scaleX - m_viewBox.height();
            foreach (KoSubpath *subpath, subpaths()) {
                foreach (KoPathPoint *currPoint, *subpath) {
                    if (currPoint->point().y() >=  m_pathStretchPointY &&
                            currPoint->controlPoint1().y() >= m_pathStretchPointY &&
                            currPoint->controlPoint2().y() >= m_pathStretchPointY) {
                        currPoint->setPoint(QPointF(currPoint->point().x(), currPoint->point().y() + deltaY));
                        currPoint->setControlPoint1(QPointF(currPoint->controlPoint1().x(),
                                                            currPoint->controlPoint1().y() + deltaY));
                        currPoint->setControlPoint2(QPointF(currPoint->controlPoint2().x(),
                                                            currPoint->controlPoint2().y() + deltaY));
                        retval = true;
                    }
                }
            }
            scale = scaleX / m_viewBox.width();
        }

        notifyPointsChanged();
    }
    return retval;
}

void EnhancedPathShape::parsePathData(const QString &data)
{
    if (data.isEmpty()) {
        return;
    }

    int start = -1;
    bool separator = true;
    for (int i = 0; i < data.length(); ++i) {
        QChar ch = data.at(i);
        ushort uni_ch = ch.unicode();
        if (separator && (uni_ch == 'M' || uni_ch == 'L'
                          || uni_ch == 'C' || uni_ch == 'Z'
                          || uni_ch == 'N' || uni_ch == 'F'
                          || uni_ch == 'S' || uni_ch == 'T'
                          || uni_ch == 'U' || uni_ch == 'A'
                          || uni_ch == 'B' || uni_ch == 'W'
                          || uni_ch == 'V' || uni_ch == 'X'
                          || uni_ch == 'Y' || uni_ch == 'Q')) {
            if (start != -1) { // process last chars
                addCommand(data.mid(start, i - start), false);
            }
            start = i;
        }
        separator = ch.isSpace();
    }
    if (start < data.length()) {
        addCommand(data.mid(start));
    }
    if (start != -1) {
        updatePath(size());
    }
}

void EnhancedPathShape::setMirrorHorizontally(bool mirrorHorizontally)
{
    if (m_mirrorHorizontally != mirrorHorizontally) {
        m_mirrorHorizontally = mirrorHorizontally;
        updatePath(size());
    }

}

void EnhancedPathShape::setMirrorVertically(bool mirrorVertically)
{
    if (m_mirrorVertically != mirrorVertically) {
        m_mirrorVertically = mirrorVertically;
        updatePath(size());
    }
}

void EnhancedPathShape::shapeChanged(ChangeType type, KoShape *shape)
{
    KoParameterShape::shapeChanged(type, shape);

    if (!shape || shape == this) {
        if (type == ParentChanged || type == ParameterChanged) {
            updateTextArea();
        }
    }
}

void EnhancedPathShape::updateTextArea()
{
    if (m_textArea.size() >= 4) {
        QRectF r = m_viewBox;
        r.setLeft(evaluateConstantOrReference(m_textArea[0]));
        r.setTop(evaluateConstantOrReference(m_textArea[1]));
        r.setRight(evaluateConstantOrReference(m_textArea[2]));
        r.setBottom(evaluateConstantOrReference(m_textArea[3]));
        r = m_viewMatrix.mapRect(r).translated(m_viewBoxOffset);
        setPreferredTextRect(r);
    }
}

void EnhancedPathShape::enableResultCache(bool enable)
{
    m_resultCache.clear();
    m_cacheResults = enable;
}

void EnhancedPathShape::setPathStretchPointX(qreal pathStretchPointX)
{
    if (m_pathStretchPointX != pathStretchPointX) {
        m_pathStretchPointX = pathStretchPointX;
    }

}

void EnhancedPathShape::setPathStretchPointY(qreal pathStretchPointY)
{
    if (m_pathStretchPointY != pathStretchPointY) {
        m_pathStretchPointY = pathStretchPointY;
    }

}
