/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
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

#include "KoListLevelProperties.h"
#include "Styles_p.h"

#include <float.h>

#include <kdebug.h>

#include <KoXmlNS.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlWriter.h>
#include <KoUnit.h>
#include <KoText.h>
#include <KoImageCollection.h>
#include <KoImageData.h>
#include <KoOdfNumberDefinition.h>

class KoListLevelProperties::Private
{
public:
    StylePrivate stylesPrivate;

    void copy(Private *other) {
        stylesPrivate = other->stylesPrivate;
    }
};

KoListLevelProperties::KoListLevelProperties()
        : d(new Private())
{
}

KoListLevelProperties::KoListLevelProperties(const KoListLevelProperties &other)
        : d(new Private())
{
    d->copy(other.d);
}

KoListLevelProperties::~KoListLevelProperties()
{
    delete d;
}

int KoListLevelProperties::styleId() const
{
    return propertyInt(KoListStyle::StyleId);
}

void KoListLevelProperties::setStyleId(int id)
{
    setProperty(KoListStyle::StyleId, id);
}

void KoListLevelProperties::setProperty(int key, const QVariant &value)
{
    d->stylesPrivate.add(key, value);
}

int KoListLevelProperties::propertyInt(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (variant.isNull())
        return 0;
    return variant.toInt();
}

uint KoListLevelProperties::propertyUInt(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (variant.isNull())
        return 0;
    return variant.toUInt();
}

qulonglong KoListLevelProperties::propertyULongLong(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (variant.isNull())
        return 0;
    return variant.toULongLong();
}

qreal KoListLevelProperties::propertyDouble(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (variant.isNull())
        return 0.;
    return variant.toDouble();
}

bool KoListLevelProperties::propertyBoolean(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (variant.isNull())
        return false;
    return variant.toBool();
}

QString KoListLevelProperties::propertyString(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (variant.isNull())
        return QString();
    return qvariant_cast<QString>(variant);
}

void KoListLevelProperties::applyStyle(QTextListFormat &format) const
{
    QList<int> keys = d->stylesPrivate.keys();
    for (int i = 0; i < keys.count(); i++) {
        QVariant variant = d->stylesPrivate.value(keys[i]);
        format.setProperty(keys[i], variant);
    }
}

bool KoListLevelProperties::operator==(const KoListLevelProperties &other) const
{
    return d->stylesPrivate == other.d->stylesPrivate;
}

bool KoListLevelProperties::operator!=(const KoListLevelProperties &other) const
{
    return d->stylesPrivate != other.d->stylesPrivate;
}

void KoListLevelProperties::setListItemPrefix(const QString &prefix)
{
    setProperty(KoListStyle::ListItemPrefix, prefix);
}

QString KoListLevelProperties::listItemPrefix() const
{
    return propertyString(KoListStyle::ListItemPrefix);
}

void KoListLevelProperties::setStyle(KoListStyle::Style style)
{
    setProperty(QTextListFormat::ListStyle, (int) style);
}

KoListStyle::Style KoListLevelProperties::style() const
{
    return static_cast<KoListStyle::Style>(propertyInt(QTextListFormat::ListStyle));
}

void KoListLevelProperties::setListItemSuffix(const QString &suffix)
{
    setProperty(KoListStyle::ListItemSuffix, suffix);
}

QString KoListLevelProperties::listItemSuffix() const
{
    return propertyString(KoListStyle::ListItemSuffix);
}

void KoListLevelProperties::setStartValue(int value)
{
    setProperty(KoListStyle::StartValue, value);
}

int KoListLevelProperties::startValue() const
{
    return propertyInt(KoListStyle::StartValue);
}

void KoListLevelProperties::setLevel(int value)
{
    setProperty(KoListStyle::Level, value);
}

int KoListLevelProperties::level() const
{
    return propertyInt(KoListStyle::Level);
}

void KoListLevelProperties::setDisplayLevel(int level)
{
    setProperty(KoListStyle::DisplayLevel, level);
}

int KoListLevelProperties::displayLevel() const
{
    return propertyInt(KoListStyle::DisplayLevel);
}

void KoListLevelProperties::setCharacterStyleId(int id)
{
    setProperty(KoListStyle::CharacterStyleId, id);
}

int KoListLevelProperties::characterStyleId() const
{
    return propertyInt(KoListStyle::CharacterStyleId);
}

void KoListLevelProperties::setBulletCharacter(QChar character)
{
    setProperty(KoListStyle::BulletCharacter, (int) character.unicode());
}

QChar KoListLevelProperties::bulletCharacter() const
{
    return propertyInt(KoListStyle::BulletCharacter);
}

void KoListLevelProperties::setRelativeBulletSize(int percent)
{
    setProperty(KoListStyle::BulletSize, percent);
}

int KoListLevelProperties::relativeBulletSize() const
{
    return propertyInt(KoListStyle::BulletSize);
}

void KoListLevelProperties::setAlignment(Qt::Alignment align)
{
    setProperty(KoListStyle::Alignment, static_cast<int>(align));
}

Qt::Alignment KoListLevelProperties::alignment() const
{
    return static_cast<Qt::Alignment>(propertyInt(KoListStyle::Alignment));
}

void KoListLevelProperties::setMinimumWidth(qreal width)
{
    setProperty(KoListStyle::MinimumWidth, width);
}

qreal KoListLevelProperties::minimumWidth() const
{
    return propertyDouble(KoListStyle::MinimumWidth);
}

void KoListLevelProperties::setWidth(qreal width)
{
    setProperty(KoListStyle::Width, width);
}

qreal KoListLevelProperties::width() const
{
    return propertyDouble(KoListStyle::Width);
}

void KoListLevelProperties::setHeight(qreal height)
{
    setProperty(KoListStyle::Height, height);
}

qreal KoListLevelProperties::height() const
{
    return propertyDouble(KoListStyle::Height);
}

void KoListLevelProperties::setBulletImage(KoImageData *imageData)
{
    setProperty(KoListStyle::BulletImageKey, imageData->key());
}

KoListLevelProperties & KoListLevelProperties::operator=(const KoListLevelProperties & other)
{
    d->copy(other.d);
    return *this;
}

void KoListLevelProperties::setListId(KoListStyle::ListIdType listId)
{
    setProperty(KoListStyle::ListId, listId);
}

KoListStyle::ListIdType KoListLevelProperties::listId() const
{
    if (sizeof(KoListStyle::ListIdType) == sizeof(uint))
        return propertyUInt(KoListStyle::ListId);
    else
        return propertyULongLong(KoListStyle::ListId);
}

bool KoListLevelProperties::letterSynchronization() const
{
    return propertyBoolean(KoListStyle::LetterSynchronization);
}

void KoListLevelProperties::setLetterSynchronization(bool on)
{
    setProperty(KoListStyle::LetterSynchronization, on);
}

void KoListLevelProperties::setContinueNumbering(bool enable)
{
    setProperty(KoListStyle::ContinueNumbering, enable);
}

bool KoListLevelProperties::continueNumbering() const
{
    return propertyBoolean(KoListStyle::ContinueNumbering);
}

void KoListLevelProperties::setIndent(qreal value)
{
    setProperty(KoListStyle::Indent, value);
}

qreal KoListLevelProperties::indent() const
{
    return propertyDouble(KoListStyle::Indent);
}

void KoListLevelProperties::setMinimumDistance(qreal value)
{
    setProperty(KoListStyle::MinimumDistance, value);
}

qreal KoListLevelProperties::minimumDistance() const
{
    return propertyDouble(KoListStyle::MinimumDistance);
}

// static
KoListLevelProperties KoListLevelProperties::fromTextList(QTextList *list)
{
    KoListLevelProperties llp;
    if (!list) {
        llp.setStyle(KoListStyle::None);
        return llp;
    }
    llp.d->stylesPrivate = list->format().properties();
    return llp;
}

void KoListLevelProperties::loadOdf(KoShapeLoadingContext& scontext, const KoXmlElement& style)
{
    KoOdfLoadingContext &context = scontext.odfLoadingContext();

    // The text:level attribute specifies the level of the number list
    // style. It can be used on all list-level styles.
    const int level = qMax(1, style.attributeNS(KoXmlNS::text, "level", QString()).toInt());
    // The text:display-levels attribute specifies the number of
    // levels whose numbers are displayed at the current level.
    const QString displayLevel = style.attributeNS(KoXmlNS::text,
                                 "display-levels", QString());

    if (style.localName() == "list-level-style-bullet") {   // list with bullets

        //1.6: KoParagCounter::loadOasisListStyle
        QString bulletChar = style.isNull() ? QString() : style.attributeNS(KoXmlNS::text, "bullet-char", QString());
        kDebug(32500) << "style.localName()=" << style.localName() << "level=" << level << "displayLevel=" << displayLevel << "bulletChar=" << bulletChar;
        if (bulletChar.isEmpty()) {  // list without any visible bullets
            setStyle(KoListStyle::CustomCharItem);
            setBulletCharacter(QChar());
        } else { // try to determinate the bullet we should use
            switch (bulletChar[0].unicode()) {
            case 0x2022: // bullet, a small disc -> circle
                //TODO use BulletSize to differ between small and large discs
                setStyle(KoListStyle::DiscItem);
                break;
            case 0x25CF: // black circle, large disc -> disc
            case 0xF0B7: // #113361
                setStyle(KoListStyle::DiscItem);
                break;
            case 0xE00C: // losange => rhombus
                setStyle(KoListStyle::RhombusItem);
                break;
            case 0xE00A: // square. Not in OASIS (reserved Unicode area!), but used in both OOo and kotext.
                setStyle(KoListStyle::SquareItem);
                break;
            case 0x27A2: // two-colors right-pointing triangle
                setStyle(KoListStyle::RightArrowHeadItem);
                break;
            case 0x2794: // arrow to right
                setStyle(KoListStyle::RightArrowItem);
                break;
            case 0x2714: // checkmark
                setStyle(KoListStyle::HeavyCheckMarkItem);
                break;
            case 0x2d: // minus
                setStyle(KoListStyle::CustomCharItem);
                break;
            case 0x2717: // cross
                setStyle(KoListStyle::BallotXItem);
                break;
            default:
                QChar customBulletChar = bulletChar[0];
                kDebug(32500) << "Unhandled bullet code 0x" << QString::number((uint)customBulletChar.unicode(), 16);
                kDebug(32500) << "Should use the style =>" << style.attributeNS(KoXmlNS::text, "style-name", QString()) << "<=";
                setStyle(KoListStyle::CustomCharItem);
                /*
                QString customBulletFont;
                // often StarSymbol when it comes from OO; doesn't matter, Qt finds it in another font if needed.
                if ( listStyleProperties.hasAttributeNS( KoXmlNS::style, "font-name" ) )
                {
                    customBulletFont = listStyleProperties.attributeNS( KoXmlNS::style, "font-name", QString::null );
                    kDebug(32500) <<"customBulletFont style:font-name =" << listStyleProperties.attributeNS( KoXmlNS::style,"font-name", QString::null );
                }
                else if ( listStyleTextProperties.hasAttributeNS( KoXmlNS::fo, "font-family" ) )
                {
                    customBulletFont = listStyleTextProperties.attributeNS( KoXmlNS::fo, "font-family", QString::null );
                    kDebug(32500) <<"customBulletFont fo:font-family =" << listStyleTextProperties.attributeNS( KoXmlNS::fo,"font-family", QString::null );
                }
                // ## TODO in fact we're supposed to read it from the style pointed to by text:style-name
                */
//                     setStyle(KoListStyle::BoxItem); //fallback
                break;
            } // switch
            setBulletCharacter(bulletChar[0]);
        }

    } else if (style.localName() == "list-level-style-number" || style.localName() == "outline-level-style") { // it's a numbered list

        KoOdfNumberDefinition numberDefinition;
        numberDefinition.loadOdf(style);

        switch(numberDefinition.formatSpecification()) {
        case KoOdfNumberDefinition::Empty:
            setStyle(KoListStyle::CustomCharItem);
            setBulletCharacter(QChar());
        case KoOdfNumberDefinition::AlphabeticLowerCase:
            setStyle(KoListStyle::AlphaLowerItem);
            break;
        case KoOdfNumberDefinition::AlphabeticUpperCase:
            setStyle(KoListStyle::UpperAlphaItem);
            break;
        case KoOdfNumberDefinition::RomanLowerCase:
            setStyle(KoListStyle::RomanLowerItem);
            break;
        case KoOdfNumberDefinition::RomanUpperCase:
            setStyle(KoListStyle::UpperRomanItem);
            break;
        case KoOdfNumberDefinition::Numeric:
        default:
            setStyle(KoListStyle::DecimalItem);
        }

        if (!numberDefinition.prefix().isNull()) {
            setListItemPrefix(numberDefinition.prefix());
        }

        if (!numberDefinition.suffix().isNull()) {
            setListItemSuffix(numberDefinition.suffix());
        }
        const QString startValue = style.attributeNS(KoXmlNS::text, "start-value", QString("1"));
        setStartValue(startValue.toInt());
    }
    else if (style.localName() == "list-level-style-image") {   // list with image
        setStyle(KoListStyle::ImageItem);
        KoImageCollection *imageCollection = scontext.imageCollection();
        const QString href = style.attribute("href");
        if(imageCollection) {
            if (!href.isEmpty()) {
                KoStore *store = context.store();
                setBulletImage(imageCollection->createImageData(href, store));
            } else {
                // check if we have an office:binary data element containing the image data
                const KoXmlElement &binaryData(KoXml::namedItemNS(style, KoXmlNS::office, "binary-data"));
                if (!binaryData.isNull()) {
                    QImage image;
                    if (image.loadFromData(QByteArray::fromBase64(binaryData.text().toLatin1()))) {
                        setBulletImage(imageCollection->createImageData(image));
                    }
                }
            }
        }
    }
    else { // if not defined, we have do nothing
        kDebug(32500) << "stylename else:" << style.localName() << "level=" << level << "displayLevel=" << displayLevel;
        setStyle(KoListStyle::DecimalItem);
        setListItemSuffix(".");
    }

    setLevel(level);
    if (!displayLevel.isEmpty())
        setDisplayLevel(displayLevel.toInt());

    KoXmlElement property;
    forEachElement(property, style) {
        if (property.namespaceURI() != KoXmlNS::style)
            continue;
        const QString localName = property.localName();
        if (localName == "list-level-properties") {
            if (property.hasAttributeNS(KoXmlNS::text, "space-before"))
                setIndent(KoUnit::parseValue(property.attributeNS(KoXmlNS::text, "space-before")));

            if (property.hasAttributeNS(KoXmlNS::text, "min-label-width"))
                setMinimumWidth(KoUnit::parseValue(property.attributeNS(KoXmlNS::text, "min-label-width")));

            if (property.hasAttributeNS(KoXmlNS::fo, "text-align"))
                setAlignment(KoText::alignmentFromString(property.attributeNS(KoXmlNS::fo, "text-align")));

            if (property.hasAttributeNS(KoXmlNS::text, "min-label-distance"))
                setMinimumDistance(KoUnit::parseValue(property.attributeNS(KoXmlNS::text, "min-label-distance")));

            if (property.hasAttributeNS(KoXmlNS::fo, "width"))
                setWidth(KoUnit::parseValue(property.attributeNS(KoXmlNS::fo, "width")));

            if (property.hasAttributeNS(KoXmlNS::fo, "height"))
                setHeight(KoUnit::parseValue(property.attributeNS(KoXmlNS::fo, "height")));
        } else if (localName == "text-properties") {
            // TODO
        }
    }
}

static QString toPoint(qreal number)
{
    QString str;
    str.setNum(number, 'f', DBL_DIG);
    str += "pt";
    return str;
}

void KoListLevelProperties::saveOdf(KoXmlWriter *writer) const
{
    bool isNumber = false;
    switch (d->stylesPrivate.value(QTextListFormat::ListStyle).toInt()) {
    case KoListStyle::DecimalItem:
    case KoListStyle::AlphaLowerItem:
    case KoListStyle::UpperAlphaItem:
    case KoListStyle::RomanLowerItem:
    case KoListStyle::UpperRomanItem:
        isNumber = true;
        break;
    }
    if (isNumber)
        writer->startElement("text:list-level-style-number");
    else
        writer->startElement("text:list-level-style-bullet");

    // These apply to bulleted and numbered lists
    if (d->stylesPrivate.contains(KoListStyle::Level))
        writer->addAttribute("text:level", d->stylesPrivate.value(KoListStyle::Level).toInt());
    if (d->stylesPrivate.contains(KoListStyle::ListItemPrefix))
        writer->addAttribute("style:num-prefix", d->stylesPrivate.value(KoListStyle::ListItemPrefix).toString());
    if (d->stylesPrivate.contains(KoListStyle::ListItemSuffix))
        writer->addAttribute("style:num-suffix", d->stylesPrivate.value(KoListStyle::ListItemSuffix).toString());

    if (isNumber) {
        if (d->stylesPrivate.contains(KoListStyle::StartValue))
            writer->addAttribute("text:start-value", d->stylesPrivate.value(KoListStyle::StartValue).toInt());
        if (d->stylesPrivate.contains(KoListStyle::DisplayLevel))
            writer->addAttribute("text:display-levels", d->stylesPrivate.value(KoListStyle::DisplayLevel).toInt());

        QChar format;
        switch (style()) {
        case KoListStyle::DecimalItem:      format = '1'; break;
        case KoListStyle::AlphaLowerItem:   format = 'a'; break;
        case KoListStyle::UpperAlphaItem:   format = 'A'; break;
        case KoListStyle::RomanLowerItem:   format = 'i'; break;
        case KoListStyle::UpperRomanItem:   format = 'I'; break;
        default: break;
        }
        writer->addAttribute("style:num-format", format);
    } else {
        int bullet;
        if (d->stylesPrivate.contains(KoListStyle::BulletCharacter)) {
            bullet = d->stylesPrivate.value(KoListStyle::BulletCharacter).toInt();
        } else { // try to determine the bullet character from the style
            switch (style()) {
            case KoListStyle::DiscItem:             bullet = 0x2022; break;
            case KoListStyle::RhombusItem:          bullet = 0xE00C; break;
            case KoListStyle::SquareItem:           bullet = 0xE00A; break;
            case KoListStyle::RightArrowHeadItem:   bullet = 0x27A2; break;
            case KoListStyle::RightArrowItem:       bullet = 0x2794; break;
            case KoListStyle::HeavyCheckMarkItem:   bullet = 0x2714; break;
            case KoListStyle::BallotXItem:          bullet = 0x2717; break;
            default:                                bullet = 0x25CF; break;
            }
        }
        writer->addAttribute("text:bullet-char", QChar(bullet));
    }

    writer->startElement("style:list-level-properties", false);

    if (d->stylesPrivate.contains(KoListStyle::Indent))
        writer->addAttribute("text:space-before", toPoint(indent()));

    if (d->stylesPrivate.contains(KoListStyle::MinimumWidth))
        writer->addAttribute("text:min-label-width", toPoint(minimumWidth()));

    if (d->stylesPrivate.contains(KoListStyle::Alignment))
        writer->addAttribute("fo:text-align", KoText::alignmentToString(alignment()));

    if (d->stylesPrivate.contains(KoListStyle::MinimumDistance))
        writer->addAttribute("text:min-label-distance", toPoint(minimumDistance()));

    writer->endElement(); // list-level-properties

//   kDebug(32500) << "Key KoListStyle::ListItemPrefix :" << d->stylesPrivate.value(KoListStyle::ListItemPrefix);
//   kDebug(32500) << "Key KoListStyle::ListItemSuffix :" << d->stylesPrivate.value(KoListStyle::ListItemSuffix);
//   kDebug(32500) << "Key KoListStyle::CharacterStyleId :" << d->stylesPrivate.value(KoListStyle::CharacterStyleId);
//   kDebug(32500) << "Key KoListStyle::BulletSize :" << d->stylesPrivate.value(KoListStyle::BulletSize);
//   kDebug(32500) << "Key KoListStyle::Alignment :" << d->stylesPrivate.value(KoListStyle::Alignment);
//   kDebug(32500) << "Key KoListStyle::LetterSynchronization :" << d->stylesPrivate.value(KoListStyle::LetterSynchronization);

    writer->endElement();
}
