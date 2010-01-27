/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <casper.boemann@kogmbh.com>
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
#include "KoSectionStyle.h"
#include "KoStyleManager.h"
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include "Styles_p.h"
#include "KoTextDocument.h"

#include <KDebug>

#include <QTextFrame>
#include <QTextFrameFormat>
#include <QTextCursor>
#include <QBuffer>

#include <KoUnit.h>
#include <KoStyleStack.h>
#include <KoOdfLoadingContext.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoBorder.h>

class KoSectionStyle::Private
{
public:
    Private() : parentStyle(0) {}

    ~Private() {
    }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate.add(key, value);
    }
    int propertyInt(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return 0;
        return variant.toInt();
    }
    bool propertyBoolean(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return false;
        return variant.toBool();
    }
    qreal propertyDouble(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return 0.0;
        return variant.toDouble();
    }
    QColor propertyColor(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return QColor();
        return variant.value<QColor>();
    }

    QString name;
    KoSectionStyle *parentStyle;
    StylePrivate stylesPrivate;
};

KoSectionStyle::KoSectionStyle(QObject *parent)
        : QObject(parent), d(new Private())
{
}

KoSectionStyle::KoSectionStyle(const QTextFrameFormat &sectionFormat, QObject *parent)
        : QObject(parent),
        d(new Private())
{
    d->stylesPrivate = sectionFormat.properties();
}

KoSectionStyle::~KoSectionStyle()
{
    delete d;
}

void KoSectionStyle::setParentStyle(KoSectionStyle *parent)
{
    d->parentStyle = parent;
}

void KoSectionStyle::setProperty(int key, const QVariant &value)
{
    if (d->parentStyle) {
        QVariant var = d->parentStyle->value(key);
        if (!var.isNull() && var == value) { // same as parent, so its actually a reset.
            d->stylesPrivate.remove(key);
            return;
        }
    }
    d->stylesPrivate.add(key, value);
}

void KoSectionStyle::remove(int key)
{
    d->stylesPrivate.remove(key);
}

QVariant KoSectionStyle::value(int key) const
{
    QVariant var = d->stylesPrivate.value(key);
    if (var.isNull() && d->parentStyle)
        var = d->parentStyle->value(key);
    return var;
}

bool KoSectionStyle::hasProperty(int key) const
{
    return d->stylesPrivate.contains(key);
}

void KoSectionStyle::applyStyle(QTextFrameFormat &format) const
{
    if (d->parentStyle) {
        d->parentStyle->applyStyle(format);
    }
    QList<int> keys = d->stylesPrivate.keys();
    for (int i = 0; i < keys.count(); i++) {
        QVariant variant = d->stylesPrivate.value(keys[i]);
        format.setProperty(keys[i], variant);
    }
}

void KoSectionStyle::applyStyle(QTextFrame &section) const
{
    QTextFrameFormat format = section.frameFormat();
    applyStyle(format);
    section.setFrameFormat(format);
}

void KoSectionStyle::unapplyStyle(QTextFrame &section) const
{
    if (d->parentStyle)
        d->parentStyle->unapplyStyle(section);

    QTextFrameFormat format = section.frameFormat();

    QList<int> keys = d->stylesPrivate.keys();
    for (int i = 0; i < keys.count(); i++) {
        QVariant variant = d->stylesPrivate.value(keys[i]);
        if (variant == format.property(keys[i]))
            format.clearProperty(keys[i]);
    }
    section.setFrameFormat(format);
}

void KoSectionStyle::setLeftMargin(qreal margin)
{
    setProperty(QTextFormat::BlockLeftMargin, margin);
}

qreal KoSectionStyle::leftMargin() const
{
    return d->propertyDouble(QTextFormat::BlockLeftMargin);
}

void KoSectionStyle::setRightMargin(qreal margin)
{
    setProperty(QTextFormat::BlockRightMargin, margin);
}

qreal KoSectionStyle::rightMargin() const
{
    return d->propertyDouble(QTextFormat::BlockRightMargin);
}

KoSectionStyle *KoSectionStyle::parentStyle() const
{
    return d->parentStyle;
}

QString KoSectionStyle::name() const
{
    return d->name;
}

void KoSectionStyle::setName(const QString &name)
{
    if (name == d->name)
        return;
    d->name = name;
    emit nameChanged(name);
}

int KoSectionStyle::styleId() const
{
    return d->propertyInt(StyleId);
}

void KoSectionStyle::setStyleId(int id)
{
    setProperty(StyleId, id);
}


KoText::Direction KoSectionStyle::textProgressionDirection() const
{
    return static_cast<KoText::Direction>(d->propertyInt(TextProgressionDirection));
}

void KoSectionStyle::setTextProgressionDirection(KoText::Direction dir)
{
    setProperty(TextProgressionDirection, dir);
}

void KoSectionStyle::setBackground(const QBrush &brush)
{
    d->setProperty(QTextFormat::BackgroundBrush, brush);
}

void KoSectionStyle::clearBackground()
{
    d->stylesPrivate.remove(QTextCharFormat::BackgroundBrush);
}

QBrush KoSectionStyle::background() const
{
    QVariant variant = d->stylesPrivate.value(QTextFormat::BackgroundBrush);

    if (variant.isNull()) {
        QBrush brush;
        return brush;
    }
    return qvariant_cast<QBrush>(variant);
}

void KoSectionStyle::loadOdf(const KoXmlElement *element, KoOdfLoadingContext &context)
{
    if (element->hasAttributeNS(KoXmlNS::style, "display-name"))
        d->name = element->attributeNS(KoXmlNS::style, "display-name", QString());

    if (d->name.isEmpty()) // if no style:display-name is given us the style:name
        d->name = element->attributeNS(KoXmlNS::style, "name", QString());

    context.styleStack().save();
    // Load all parents - only because we don't support inheritance.
    QString family = element->attributeNS(KoXmlNS::style, "family", "section");
    context.addStyles(element, family.toLocal8Bit().constData());   // Load all parents - only because we don't support inheritance.

    context.styleStack().setTypeProperties("section");   // load all style attributes from "style:section-properties"

    KoStyleStack &styleStack = context.styleStack();

    // in 1.6 this was defined at KoParagLayout::loadOasisParagLayout(KoParagLayout&, KoOasisContext&)

    if (styleStack.hasProperty(KoXmlNS::style, "writing-mode")) {     // http://www.w3.org/TR/2004/WD-xsl11-20041216/#writing-mode
        QString writingMode = styleStack.property(KoXmlNS::style, "writing-mode");
        setTextProgressionDirection(KoText::directionFromString(writingMode));
    }

    // Indentation (margin)
    bool hasMarginLeft = styleStack.hasProperty(KoXmlNS::fo, "margin-left");
    bool hasMarginRight = styleStack.hasProperty(KoXmlNS::fo, "margin-right");
    if (hasMarginLeft)
        setLeftMargin(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "margin-left")));
    if (hasMarginRight)
        setRightMargin(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "margin-right")));


    // The fo:background-color attribute specifies the background color of a paragraph.
    if (styleStack.hasProperty(KoXmlNS::fo, "background-color")) {
        const QString bgcolor = styleStack.property(KoXmlNS::fo, "background-color");
        QBrush brush = background();
        if (bgcolor == "transparent")
            brush.setStyle(Qt::NoBrush);
        else {
            if (brush.style() == Qt::NoBrush)
                brush.setStyle(Qt::SolidPattern);
            brush.setColor(bgcolor); // #rrggbb format
        }
        setBackground(brush);
    }

    styleStack.restore();
}


void KoSectionStyle::copyProperties(const KoSectionStyle *style)
{
    d->stylesPrivate = style->d->stylesPrivate;
    setName(style->name()); // make sure we emit property change
    d->parentStyle = style->d->parentStyle;
}

KoSectionStyle *KoSectionStyle::clone(QObject *parent) const
{
    KoSectionStyle *newStyle = new KoSectionStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}

bool KoSectionStyle::operator==(const KoSectionStyle &other) const
{
    return other.d->stylesPrivate == d->stylesPrivate;
}

void KoSectionStyle::removeDuplicates(const KoSectionStyle &other)
{
    d->stylesPrivate.removeDuplicates(other.d->stylesPrivate);
}

void KoSectionStyle::saveOdf(KoGenStyle &style)
{
    // only custom style have a displayname. automatic styles don't have a name set.
    if (!d->name.isEmpty() && !style.isDefaultStyle()) {
        style.addAttribute("style:display-name", d->name);
    }

    QList<int> keys = d->stylesPrivate.keys();
    foreach(int key, keys) {
        if (key == KoSectionStyle::TextProgressionDirection) {
            int directionValue = 0;
            bool ok = false;
            directionValue = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                QString direction;
                if (directionValue == KoText::LeftRightTopBottom)
                    direction = "lr-tb";
                else if (directionValue == KoText::RightLeftTopBottom)
                    direction = "rl-tb";
                else if (directionValue == KoText::TopBottomRightLeft)
                    direction = "tb-lr";
                else if (directionValue == KoText::InheritDirection)
                    direction = "page";
                if (!direction.isEmpty())
                    style.addProperty("style:writing-mode", direction, KoGenStyle::DefaultType);
            }
        } else if (key == QTextFormat::BackgroundBrush) {
            QBrush backBrush = background();
            if (backBrush.style() != Qt::NoBrush)
                style.addProperty("fo:background-color", backBrush.color().name(), KoGenStyle::ParagraphType);
            else
                style.addProperty("fo:background-color", "transparent", KoGenStyle::DefaultType);
        } else if (key == QTextFormat::BlockLeftMargin) {
            style.addPropertyPt("fo:margin-left", leftMargin(), KoGenStyle::DefaultType);
        } else if (key == QTextFormat::BlockRightMargin) {
            style.addPropertyPt("fo:margin-right", rightMargin(), KoGenStyle::DefaultType);
      }
    }
}

#include <KoSectionStyle.moc>
