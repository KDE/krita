/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009-2010 Thomas Zander <zander@kde.org>
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

EnhancedPathShape::EnhancedPathShape(const QRectF &viewBox)
: m_viewBox(viewBox), m_viewBoxOffset(0.0, 0.0), m_mirrorVertically(false), m_mirrorHorizontally(false)
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
}

void EnhancedPathShape::moveHandleAction(int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    EnhancedPathHandle *handle = m_enhancedHandles[ handleId ];
    if (handle) {
        handle->changePosition(shapeToViewbox(point));
        evaluateHandles();
    }
}

void EnhancedPathShape::updatePath(const QSizeF &)
{
    clear();

    foreach (EnhancedPathCommand *cmd, m_commands)
        cmd->execute();

    normalize();
}

void EnhancedPathShape::setSize(const QSizeF &newSize)
{
    QMatrix matrix(resizeMatrix(newSize));

    KoParameterShape::setSize(newSize);

    qreal scaleX = matrix.m11();
    qreal scaleY = matrix.m22();
    m_viewBoxOffset.rx() *= scaleX;
    m_viewBoxOffset.ry() *= scaleY;
    m_viewMatrix.scale(scaleX, scaleY);

    setMirroring();
}


QPointF EnhancedPathShape::normalize()
{
    QPointF offset = KoPathShape::normalize();
    m_viewBoxOffset -= offset;

    return offset;
}

void EnhancedPathShape::evaluateHandles()
{
    const int handleCount = m_enhancedHandles.count();
    QList<QPointF> handles;
    for (int i = 0; i < handleCount; ++i)
        handles.append(viewboxToShape(m_enhancedHandles[i]->position()));
    setHandles(handles);
}

qreal EnhancedPathShape::evaluateReference(const QString &reference)
{
    if (reference.isEmpty())
        return 0.0;

    QChar c = reference[0];

    qreal res = 0.0;

    switch(c.toAscii()) {
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
        FormulaStore::const_iterator formulaIt = m_formulae.constFind(fname);
        if (formulaIt != m_formulae.constEnd())
        {
            EnhancedPathFormula * formula = formulaIt.value();
            if (formula)
                res = formula->evaluate();
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

void EnhancedPathShape::modifyReference(const QString &reference, qreal value)
{
    if (reference.isEmpty())
        return;

    QChar c = reference[0];

    if (c.toAscii() == '$') {
        bool success = false;
        int modifierIndex = reference.mid(1).toInt(&success);
        if (modifierIndex >= 0 && modifierIndex < m_modifiers.count())
            m_modifiers[modifierIndex] = value;
    }
}

EnhancedPathParameter * EnhancedPathShape::parameter(const QString & text)
{
    Q_ASSERT(! text.isEmpty());

    ParameterStore::const_iterator parameterIt = m_parameters.constFind(text);
    if (parameterIt != m_parameters.constEnd()) {
        return parameterIt.value();
    } else {
        EnhancedPathParameter *parameter = 0;
        QChar c = text[0];
        if (c.toAscii() == '$' || c.toAscii() == '?') {
            parameter = new EnhancedPathReferenceParameter(text, this);
        } else {
            if (c.isDigit()) {
                bool success = false;
                qreal constant = text.toDouble(&success);
                if (success)
                    parameter = new EnhancedPathConstantParameter(constant, this);
            } else {
                Identifier identifier = EnhancedPathNamedParameter::identifierFromString(text);
                if (identifier != IdentifierUnknown)
                    parameter = new EnhancedPathNamedParameter(identifier, this);
            }
        }

        if (parameter)
            m_parameters[text] = parameter;

        return parameter;
    }
}

void EnhancedPathShape::addFormula(const QString &name, const QString &formula)
{
    if (name.isEmpty() || formula.isEmpty())
        return;

    m_formulae[name] = new EnhancedPathFormula(formula, this);
}

void EnhancedPathShape::addHandle(const QMap<QString,QVariant> &handle)
{
    if (handle.isEmpty())
        return;

    if (! handle.contains("draw:handle-position"))
        return;
    QVariant position = handle.value("draw:handle-position");

    QStringList tokens = position.toString().simplified().split(' ');
    if (tokens.count() < 2)
        return;

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
            if (minRadius.isValid() && maxRadius.isValid())
                newHandle->setRadiusRange(parameter(minRadius.toString()), parameter(maxRadius.toString()));
        }
    } else {
        QVariant minX = handle.value("draw:handle-range-x-minimum");
        QVariant maxX = handle.value("draw:handle-range-x-maximum");
        if (minX.isValid() && maxX.isValid())
            newHandle->setRangeX(parameter(minX.toString()), parameter(maxX.toString()));

        QVariant minY = handle.value("draw:handle-range-y-minimum");
        QVariant maxY = handle.value("draw:handle-range-y-maximum");
        if (minY.isValid() && maxY.isValid())
            newHandle->setRangeY(parameter(minY.toString()), parameter(maxY.toString()));
    }

    m_enhancedHandles.append(newHandle);

    evaluateHandles();
}

void EnhancedPathShape::addModifiers(const QString &modifiers)
{
    if (modifiers.isEmpty())
        return;

    QStringList tokens = modifiers.simplified().split(' ');
    int tokenCount = tokens.count();
    for (int i = 0; i < tokenCount; ++i)
       m_modifiers.append(tokens[i].toDouble());
}

void EnhancedPathShape::addCommand(const QString &command)
{
    addCommand(command, true);
}

void EnhancedPathShape::addCommand(const QString &command, bool triggerUpdate)
{
    QString commandStr = command.simplified();
    if (commandStr.isEmpty())
        return;

    // the first character is the command
    EnhancedPathCommand *cmd = new EnhancedPathCommand(commandStr[0], this);

    // strip command char
    commandStr = commandStr.mid(1).simplified();

    // now parse the command parameters
    if (!commandStr.isEmpty()) {
        QStringList tokens = commandStr.split(' ');
        for (int i = 0; i < tokens.count(); ++i)
            cmd->addParameter(parameter(tokens[i]));
    }
    m_commands.append(cmd);

    if (triggerUpdate)
        updatePath(size());
}

const QRectF & EnhancedPathShape::viewBox() const
{
    return m_viewBox;
}

QPointF EnhancedPathShape::shapeToViewbox(const QPointF &point) const
{
    //NOTE: m_flipMatrix doesn't need to be inverted since when we flip twice the efect of the flip is inverted.
    return m_viewMatrix.inverted().map( m_flipMatrix.map(point)-m_viewBoxOffset );
}

QPointF EnhancedPathShape::viewboxToShape(const QPointF &point) const
{
    return m_flipMatrix.map(m_viewMatrix.map(point) + m_viewBoxOffset);
}

qreal EnhancedPathShape::viewboxToShape(qreal value) const
{
    return m_flipMatrix.map(m_viewMatrix.map(QPointF(value, value))).x();
}

void EnhancedPathShape::saveOdf(KoShapeSavingContext &context) const
{
    if (isParametricShape()) {
        context.xmlWriter().startElement("draw:custom-shape");
        saveOdfAttributes(context, OdfAllAttributes);

        context.xmlWriter().startElement("draw:enhanced-geometry");
        context.xmlWriter().addAttribute("svg:viewBox", QString("%1 %2 %3 %4").arg(m_viewBox.x()).arg(m_viewBox.y()).arg(m_viewBox.width()).arg(m_viewBox.height()));

        QString modifiers;
        foreach (qreal modifier, m_modifiers)
            modifiers += QString::number(modifier) + ' ';
        context.xmlWriter().addAttribute("draw:modifiers", modifiers.trimmed());

        QString path;
        foreach (EnhancedPathCommand * c, m_commands)
            path += c->toString() + ' ';
        context.xmlWriter().addAttribute("draw:enhanced-path", path.trimmed());

        FormulaStore::const_iterator i = m_formulae.constBegin();
        for (; i != m_formulae.constEnd(); ++i) {
            context.xmlWriter().startElement("draw:equation");
            context.xmlWriter().addAttribute("draw:name", i.key());
            context.xmlWriter().addAttribute("draw:formula", i.value()->toString());
            context.xmlWriter().endElement(); // draw:equation
        }

        foreach (EnhancedPathHandle * handle, m_enhancedHandles)
            handle->saveOdf(context);

        context.xmlWriter().endElement(); // draw:enhanced-geometry
        saveOdfCommonChildElements(context);
        context.xmlWriter().endElement(); // draw:custom-shape
    } else {
        KoPathShape::saveOdf(context);
    }
}

bool EnhancedPathShape::loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context)
{
    reset();

    KoXmlElement child;
    forEachElement(child, element) {
        if (child.localName() == "enhanced-geometry" && child.namespaceURI() == KoXmlNS::draw) {
            // load the viewbox
            QRectF viewBox = loadOdfViewbox(child);
            if (! viewBox.isEmpty())
                m_viewBox = viewBox;

            // load the modifiers
            QString modifiers = child.attributeNS(KoXmlNS::draw, "modifiers", "");
            if (! modifiers.isEmpty()) {
                addModifiers(modifiers);
            }

            setMirrorHorizontally( child.attributeNS(KoXmlNS::draw, "mirror-horizontal") == "true");
            setMirrorVertically( child.attributeNS(KoXmlNS::draw, "mirror-vertical") == "true");

            KoXmlElement grandChild;
            forEachElement(grandChild, child) {
                if (grandChild.namespaceURI() != KoXmlNS::draw)
                    continue;
                if (grandChild.localName() == "equation") {
                    QString name = grandChild.attributeNS(KoXmlNS::draw, "name");
                    QString formula = grandChild.attributeNS(KoXmlNS::draw, "formula");
                    addFormula(name, formula);
                } else if (grandChild.localName() == "handle") {
                    EnhancedPathHandle * handle = new EnhancedPathHandle(this);
                    if (handle->loadOdf(grandChild)) {
                        m_enhancedHandles.append(handle);
                        evaluateHandles();
                    } else {
                        delete handle;
                    }
                }

            }
            // load the enhanced path data
            QString path = child.attributeNS(KoXmlNS::draw, "enhanced-path", "");
#ifndef NWORKAROUND_ODF_BUGS
            KoOdfWorkaround::fixEnhancedPath(path, child, context);
#endif
            if (!path.isEmpty()) {
                parsePathData(path);
            }
        }
    }

    QPointF pos;
    pos.setX(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x", QString())));
    pos.setY(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y", QString())));
    setPosition(pos);
    normalize();

    QSizeF size;
    size.setWidth(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "width", QString())));
    size.setHeight(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "height", QString())));

    setSize(size);

    loadOdfAttributes(element, context, OdfMandatories | OdfTransformation | OdfAdditionalAttributes | OdfCommonChildElements);

    return true;
}

void EnhancedPathShape::parsePathData(const QString &data)
{
    if (data.isEmpty())
        return;

    int start = -1;
    bool separator = true;
    for (int i = 0; i < data.length(); ++i) {
        QChar ch = data.at(i);
        if (separator && (ch.unicode() == 'M' || ch.unicode() == 'L'
            || ch.unicode() == 'C' || ch.unicode() == 'Z'
            || ch.unicode() == 'N' || ch.unicode() == 'F'
            || ch.unicode() == 'S' || ch.unicode() == 'T'
            || ch.unicode() == 'U' || ch.unicode() == 'A'
            || ch.unicode() == 'B' || ch.unicode() == 'W'
            || ch.unicode() == 'V' || ch.unicode() == 'X'
            || ch.unicode() == 'Y' || ch.unicode() == 'Q')) {
            if (start != -1) { // process last chars
                addCommand(data.mid(start, i - start));
            }
            start = i;
        }
        separator = ch.isSpace();
    }
    if (start < data.length())
        addCommand(data.mid(start));
    if (start != -1)
        updatePath(size());
}

void EnhancedPathShape::setMirrorHorizontally(bool mirrorHorizontally)
{
    if( m_mirrorHorizontally != mirrorHorizontally) {
        m_mirrorHorizontally = mirrorHorizontally;
//         setMirroring();
    }
}

void EnhancedPathShape::setMirrorVertically(bool mirrorVertically)
{
    if( m_mirrorVertically != mirrorVertically) {
        m_mirrorVertically = mirrorVertically;
//         setMirroring();
    }
}

void EnhancedPathShape::setMirroring()
{
    qreal centerX = size().width() * 0.5;
    qreal centerY = size().height() * 0.5;

    m_flipMatrix.reset();
    m_flipMatrix.translate(centerX, centerY);
    m_flipMatrix.scale(m_mirrorHorizontally? -1.0 : 1.0, m_mirrorVertically? -1.0 : 1.0);
    m_flipMatrix.translate(-centerX, -centerY);
}
