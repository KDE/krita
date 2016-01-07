/* This file is part of the KDE project
 * Copyright (C) 2007,2010,2011 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *   Contact: Suresh Chande suresh.chande@nokia.com
 * Copyright (C) 2009-2010 Thorsten Zachmann <zachmann@kde.org>
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

#include "EnhancedPathShape.h"
#include "EnhancedPathCommand.h"
#include "EnhancedPathParameter.h"
#include "EnhancedPathHandle.h"
#include "EnhancedPathFormula.h"

#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoShapeSavingContext.h>
#include <KoUnit.h>
#include <KoOdfWorkaround.h>
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

EnhancedPathShape::~EnhancedPathShape()
{
    reset();
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
        foreach (KoSubpath *subpath, m_subpaths) {
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
        if (m_cacheResults && m_resultChache.contains(fname)) {
            res = m_resultChache.value(fname);
        } else {
            FormulaStore::const_iterator formulaIt = m_formulae.constFind(fname);
            if (formulaIt != m_formulae.constEnd()) {
                EnhancedPathFormula *formula = formulaIt.value();
                if (formula) {
                    res = formula->evaluate();
                    if (m_cacheResults) {
                        m_resultChache.insert(fname, res);
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
        if (m_viewBox.width() / m_viewBox.height() < scaleX / scaleY) {
            qreal deltaX = (scaleX * m_viewBox.height()) / scaleY - m_viewBox.width();
            foreach (KoSubpath *subpath, m_subpaths) {
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
        } else if (m_viewBox.width() / m_viewBox.height() > scaleX / scaleY) {
            qreal deltaY = (m_viewBox.width() * scaleY) / scaleX - m_viewBox.height();
            foreach (KoSubpath *subpath, m_subpaths) {
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
    }
    return retval;
}

void EnhancedPathShape::saveOdf(KoShapeSavingContext &context) const
{
    if (isParametricShape()) {
        context.xmlWriter().startElement("draw:custom-shape");

        const QSizeF currentSize = outline().boundingRect().size();

        // save the right position so that when loading we fit the viewbox
        // to the right position without getting any wrong scaling
        // -> calculate the right position from the current 0 position / viewbound ratio
        // this is e.g. the case when there is a callout that goes into negative viewbound coordinates
        QPointF topLeft = m_viewBound.topLeft();
        QPointF diff;
        if (qAbs(topLeft.x()) > 1E-5) {
            diff.setX(topLeft.x()*currentSize.width() / m_viewBound.width());
        }
        if (qAbs(topLeft.y()) > 1E-5) {
            diff.setY(topLeft.y()*currentSize.height() / m_viewBound.height());
        }

        if (diff.isNull()) {
            saveOdfAttributes(context, OdfAllAttributes & ~OdfSize);
        } else {
            //FIXME: this needs to be fixed for shapes that are transformed by rotation or skewing
            QTransform offset(context.shapeOffset(this));
            QTransform newOffset(offset);
            newOffset.translate(-diff.x(), -diff.y());
            context.addShapeOffset(this, newOffset);
            saveOdfAttributes(context, OdfAllAttributes & ~OdfSize);
            if (offset.isIdentity()) {
                context.removeShapeOffset(this);
            } else {
                context.addShapeOffset(this, offset);
            }
        }

        // save the right size so that when loading we fit the viewbox
        // to the right size without getting any wrong scaling
        // -> calculate the right size from the current size/viewbound ratio
        context.xmlWriter().addAttributePt("svg:width", currentSize.width() == 0 ? 0 : m_viewBox.width()*currentSize.width() / m_viewBound.width());
        context.xmlWriter().addAttributePt("svg:height", currentSize.height() == 0 ? 0 : m_viewBox.height()*currentSize.height() / m_viewBound.height());

        saveText(context);

        context.xmlWriter().startElement("draw:enhanced-geometry");
        context.xmlWriter().addAttribute("svg:viewBox", QString("%1 %2 %3 %4").arg(m_viewBox.x()).arg(m_viewBox.y()).arg(m_viewBox.width()).arg(m_viewBox.height()));

        if (m_pathStretchPointX != -1) {
            context.xmlWriter().addAttribute("draw:path-stretchpoint-x", m_pathStretchPointX);
        }
        if (m_pathStretchPointY != -1) {
            context.xmlWriter().addAttribute("draw:path-stretchpoint-y", m_pathStretchPointY);
        }

        if (m_mirrorHorizontally) {
            context.xmlWriter().addAttribute("draw:mirror-horizontal", "true");
        }
        if (m_mirrorVertically) {
            context.xmlWriter().addAttribute("draw:mirror-vertical", "true");
        }

        QString modifiers;
        foreach (qreal modifier, m_modifiers) {
            modifiers += QString::number(modifier) + ' ';
        }
        context.xmlWriter().addAttribute("draw:modifiers", modifiers.trimmed());

        if (m_textArea.size() >= 4) {
            context.xmlWriter().addAttribute("draw:text-areas", m_textArea.join(" "));
        }

        QString path;
        foreach (EnhancedPathCommand *c, m_commands) {
            path += c->toString() + ' ';
        }
        context.xmlWriter().addAttribute("draw:enhanced-path", path.trimmed());

        FormulaStore::const_iterator i = m_formulae.constBegin();
        for (; i != m_formulae.constEnd(); ++i) {
            context.xmlWriter().startElement("draw:equation");
            context.xmlWriter().addAttribute("draw:name", i.key());
            context.xmlWriter().addAttribute("draw:formula", i.value()->toString());
            context.xmlWriter().endElement(); // draw:equation
        }

        foreach (EnhancedPathHandle *handle, m_enhancedHandles) {
            handle->saveOdf(context);
        }

        context.xmlWriter().endElement(); // draw:enhanced-geometry
        saveOdfCommonChildElements(context);
        context.xmlWriter().endElement(); // draw:custom-shape

    } else {
        KoPathShape::saveOdf(context);
    }
}

bool EnhancedPathShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    reset();

    const KoXmlElement enhancedGeometry(KoXml::namedItemNS(element, KoXmlNS::draw, "enhanced-geometry"));
    if (!enhancedGeometry.isNull()) {

        setPathStretchPointX(enhancedGeometry.attributeNS(KoXmlNS::draw, "path-stretchpoint-x", "-1").toDouble());
        setPathStretchPointY(enhancedGeometry.attributeNS(KoXmlNS::draw, "path-stretchpoint-y", "-1").toDouble());

        // load the modifiers
        QString modifiers = enhancedGeometry.attributeNS(KoXmlNS::draw, "modifiers", "");
        if (!modifiers.isEmpty()) {
            addModifiers(modifiers);
        }

        m_textArea = enhancedGeometry.attributeNS(KoXmlNS::draw, "text-areas", "").split(' ');
        if (m_textArea.size() >= 4) {
            setResizeBehavior(TextFollowsPreferredTextRect);
        }

        KoXmlElement grandChild;
        forEachElement(grandChild, enhancedGeometry) {
            if (grandChild.namespaceURI() != KoXmlNS::draw) {
                continue;
            }
            if (grandChild.localName() == "equation") {
                QString name = grandChild.attributeNS(KoXmlNS::draw, "name");
                QString formula = grandChild.attributeNS(KoXmlNS::draw, "formula");
                addFormula(name, formula);
            } else if (grandChild.localName() == "handle") {
                EnhancedPathHandle *handle = new EnhancedPathHandle(this);
                if (handle->loadOdf(grandChild, context)) {
                    m_enhancedHandles.append(handle);
                    evaluateHandles();
                } else {
                    delete handle;
                }
            }

        }

        setMirrorHorizontally(enhancedGeometry.attributeNS(KoXmlNS::draw, "mirror-horizontal") == "true");
        setMirrorVertically(enhancedGeometry.attributeNS(KoXmlNS::draw, "mirror-vertical") == "true");

        // load the enhanced path data
        QString path = enhancedGeometry.attributeNS(KoXmlNS::draw, "enhanced-path", "");
#ifndef NWORKAROUND_ODF_BUGS
        KoOdfWorkaround::fixEnhancedPath(path, enhancedGeometry, context);
#endif
        // load the viewbox
        m_viewBox = loadOdfViewbox(enhancedGeometry);

        if (!path.isEmpty()) {
            parsePathData(path);
        }

        if (m_viewBox.isEmpty()) {
            // if there is no view box defined make it is big as the path.
            m_viewBox = m_viewBound.toAlignedRect();
        }

    }

    QSizeF size;
    size.setWidth(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "width", QString())));
    size.setHeight(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "height", QString())));
    // the viewbox is to be fitted into the size of the shape, so before setting
    // the size we just loaded // we set the viewbox to be the basis to calculate
    // the viewbox matrix from
    m_viewBound = m_viewBox;
    setSize(size);

    QPointF pos;
    pos.setX(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x", QString())));
    pos.setY(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y", QString())));
    setPosition(pos - m_viewMatrix.map(QPointF(0, 0)) - m_viewBoxOffset);

    loadOdfAttributes(element, context, OdfMandatories | OdfTransformation | OdfAdditionalAttributes | OdfCommonChildElements);

    loadText(element, context);

    return true;
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
    m_resultChache.clear();
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
