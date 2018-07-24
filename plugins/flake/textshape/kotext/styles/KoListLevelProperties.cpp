/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2008 Pierre Ducroquet <pinaraf@pinaraf.info>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2010 Nandita Suri <suri.nandita@gmail.com>
 * Copyright (C) 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>
 * Copyright (C) 2011-2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
 * Copyright (C) 2011 Mojtaba Shahi Senobari <mojtaba.shahi3000@gmail.com>
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

#include "KoTextSharedLoadingData.h"
#include "Styles_p.h"

#include <float.h>

#include "TextDebug.h"

#include <KoXmlNS.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoUnit.h>
#include <KoText.h>
#include <KoImageCollection.h>
#include <KoImageData.h>
#include <KoImageData_p.h>
#include <KoOdfNumberDefinition.h>
#include <KoGenStyle.h>
#include <KoTextSharedSavingData.h>

#include <QTextList>

class Q_DECL_HIDDEN KoListLevelProperties::Private
{
public:
    StylePrivate stylesPrivate;

    void copy(Private *other) {
        stylesPrivate = other->stylesPrivate;
    }
};

KoListLevelProperties::KoListLevelProperties()
        : QObject()
        , d(new Private())
{
    QSharedPointer<KoCharacterStyle> charStyle(new KoCharacterStyle);
    setCharacterProperties(charStyle);

    setRelativeBulletSize(100);
    setAlignmentMode(false);
    setDisplayLevel(1);
    connect(this,SIGNAL(styleChanged(int)),SLOT(onStyleChanged(int)));
}

KoListLevelProperties::KoListLevelProperties(const KoListLevelProperties &other)
        : QObject()
        , d(new Private())
{
    d->copy(other.d);
    connect(this,SIGNAL(styleChanged(int)),SLOT(onStyleChanged(int)));
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

QColor KoListLevelProperties::propertyColor(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (variant.isNull())
        return QColor(Qt::black);
    return qvariant_cast<QColor>(variant);
}

QVariant KoListLevelProperties::property(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (!variant.isNull()) {
        return variant;
    } else {
        return QVariant();
    }
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
    emit styleChanged(style);
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

void KoListLevelProperties::setCharacterProperties(QSharedPointer< KoCharacterStyle > style)
{
    setProperty(KoListStyle::CharacterProperties, QVariant::fromValue< QSharedPointer<KoCharacterStyle> >(style));
}

QSharedPointer<KoCharacterStyle> KoListLevelProperties::characterProperties() const
{
    const QVariant v = d->stylesPrivate.value(KoListStyle::CharacterProperties);
    if (v.isNull()) {
        return QSharedPointer<KoCharacterStyle>(0);
    }
    return v.value< QSharedPointer<KoCharacterStyle> >();
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
    setProperty(KoListStyle::RelativeBulletSize, percent);
}

int KoListLevelProperties::relativeBulletSize() const
{
    return propertyInt(KoListStyle::RelativeBulletSize);
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
    setProperty(KoListStyle::BulletImage, QVariant::fromValue(imageData));
}

KoImageData *KoListLevelProperties::bulletImage() const
{
    return property(KoListStyle::BulletImage).value< KoImageData * >();
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

void KoListLevelProperties::setMargin(qreal value)
{
    setProperty(KoListStyle::Margin, value);
}

qreal KoListLevelProperties::margin() const
{
    return propertyDouble(KoListStyle::Margin);
}

void KoListLevelProperties::setTextIndent(qreal value)
{
    setProperty(KoListStyle::TextIndent, value);
}

qreal KoListLevelProperties::textIndent() const
{
    return propertyDouble(KoListStyle::TextIndent);
}

void KoListLevelProperties::setAlignmentMode(bool isLabelAlignmentMode)
{
    setProperty(KoListStyle::AlignmentMode, isLabelAlignmentMode);
}

bool KoListLevelProperties::alignmentMode() const
{
    return propertyBoolean(KoListStyle::AlignmentMode);
}

void KoListLevelProperties::setTabStopPosition(qreal value)
{
    setProperty(KoListStyle::TabStopPosition,value);
}

qreal KoListLevelProperties::tabStopPosition() const
{
    return propertyDouble(KoListStyle::TabStopPosition);
}

void KoListLevelProperties::setLabelFollowedBy(KoListStyle::ListLabelFollowedBy value)
{
    setProperty(KoListStyle::LabelFollowedBy, value);
}

KoListStyle::ListLabelFollowedBy KoListLevelProperties::labelFollowedBy() const
{
    return (KoListStyle::ListLabelFollowedBy)propertyInt(KoListStyle::LabelFollowedBy);
}

void KoListLevelProperties::setOutlineList(bool isOutline)
{
    setProperty(KoListStyle::IsOutline, isOutline);
}

bool KoListLevelProperties::isOutlineList() const
{
    return propertyBoolean(KoListStyle::IsOutline);
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

void KoListLevelProperties::onStyleChanged(int key)
{
    int bullet=KoListStyle::bulletCharacter(key);
    if (bullet != 0)
        setBulletCharacter(QChar(bullet));

    //for numbered list the relative bullet size is made 100
    if (KoListStyle::isNumberingStyle(key)) {
        setRelativeBulletSize(100);
    }
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

    const QString styleName = style.attributeNS(KoXmlNS::text, "style-name", QString());
    KoCharacterStyle *cs = 0;
    if (!styleName.isEmpty()) {
//         debugText << "Should use the style =>" << styleName << "<=";

        KoSharedLoadingData *sharedData = scontext.sharedData(KOTEXT_SHARED_LOADING_ID);
        KoTextSharedLoadingData *textSharedData = 0;
        if (sharedData) {
            textSharedData = dynamic_cast<KoTextSharedLoadingData *>(sharedData);
        }
        if (textSharedData) {
            cs = textSharedData->characterStyle(styleName, context.useStylesAutoStyles());
            if (!cs) {
               warnText << "Missing KoCharacterStyle!";
            }
            else {
//                debugText << "==> cs.name:" << cs->name();
//                debugText << "==> cs.styleId:" << cs->styleId();
                setCharacterStyleId(cs->styleId());
            }
        }
    }

    if (style.localName() == "list-level-style-bullet") {   // list with bullets
        // special case bullets:
        //debugText << QChar(0x2202) << QChar(0x25CF) << QChar(0xF0B7) << QChar(0xE00C)
        //<< QChar(0xE00A) << QChar(0x27A2)<< QChar(0x2794) << QChar(0x2714) << QChar(0x2d) << QChar(0x2717);

        //1.6: KoParagCounter::loadOasisListStyle
        QString bulletChar = style.attributeNS(KoXmlNS::text, "bullet-char", QString());
//         debugText << "style.localName()=" << style.localName() << "level=" << level << "displayLevel=" << displayLevel << "bulletChar=" << bulletChar;
        if (bulletChar.isEmpty()) {  // list without any visible bullets
            setStyle(KoListStyle::CustomCharItem);
            setBulletCharacter(QChar());
        } else { // try to determinate the bullet we should use
            switch (bulletChar[0].unicode()) {
            case 0x2022: // bullet, a small disc -> circle
                setStyle(KoListStyle::Bullet);
                break;
            case 0x25CF: // black circle, large disc -> disc
                setStyle(KoListStyle::BlackCircle);
                break;
            case 0x25CB:           //white circle, no fill
                setStyle(KoListStyle::CircleItem);
                break;
            case 0x25C6: // losange => rhombus
                setStyle(KoListStyle::RhombusItem);
                break;
            case 0x25A0: // square. Not in OASIS (reserved Unicode area!), but used in both OOo and kotext.
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
                debugText << "Unhandled bullet code 0x" << QString::number((uint)customBulletChar.unicode(), 16) << bulletChar;
                debugText << "Should use the style =>" << style.attributeNS(KoXmlNS::text, "style-name", QString()) << "<=";
                setStyle(KoListStyle::CustomCharItem);
                /*
                QString customBulletFont;
                // often StarSymbol when it comes from OO; doesn't matter, Qt finds it in another font if needed.
                if ( listStyleProperties.hasAttributeNS( KoXmlNS::style, "font-name" ) )
                {
                    customBulletFont = listStyleProperties.attributeNS( KoXmlNS::style, "font-name", QString() );
                    debugText <<"customBulletFont style:font-name =" << listStyleProperties.attributeNS( KoXmlNS::style,"font-name", QString() );
                }
                else if ( listStyleTextProperties.hasAttributeNS( KoXmlNS::fo, "font-family" ) )
                {
                    customBulletFont = listStyleTextProperties.attributeNS( KoXmlNS::fo, "font-family", QString() );
                    debugText <<"customBulletFont fo:font-family =" << listStyleTextProperties.attributeNS( KoXmlNS::fo,"font-family", QString() );
                }
                // ## TODO in fact we're supposed to read it from the style pointed to by text:style-name
                */
//                     setStyle(KoListStyle::BoxItem); //fallback
                break;
            } // switch
            setBulletCharacter(bulletChar[0]);
        }
        QString size = style.attributeNS(KoXmlNS::text, "bullet-relative-size", QString());
        if (!size.isEmpty()) {
            setRelativeBulletSize(size.remove('%').toInt());
        }

    } else if (style.localName() == "list-level-style-number" || style.localName() == "outline-level-style") { // it's a numbered list

        if (style.localName() == "outline-level-style") {
            setOutlineList(true);
        }
        setRelativeBulletSize(100); //arbitrary value for numbered list

        KoOdfNumberDefinition numberDefinition;
        numberDefinition.loadOdf(style);

        switch(numberDefinition.formatSpecification()) {
        case KoOdfNumberDefinition::Empty:
            setStyle(KoListStyle::None);
            break;
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
        case KoOdfNumberDefinition::ArabicAlphabet:
                    setStyle(KoListStyle::ArabicAlphabet);
                    break;
        case KoOdfNumberDefinition::Thai:
            setStyle(KoListStyle::Thai);
            break;
        case KoOdfNumberDefinition::Abjad:
            setStyle(KoListStyle::Abjad);
            break;
        case KoOdfNumberDefinition::AbjadMinor:
            setStyle(KoListStyle::AbjadMinor);
            break;
        case KoOdfNumberDefinition::Tibetan:
            setStyle(KoListStyle::Tibetan);
            break;
        case KoOdfNumberDefinition::Telugu:
            setStyle(KoListStyle::Telugu);
            break;
        case KoOdfNumberDefinition::Tamil:
            setStyle(KoListStyle::Tamil);
            break;
        case KoOdfNumberDefinition::Oriya:
            setStyle(KoListStyle::Oriya);
            break;
        case KoOdfNumberDefinition::Malayalam:
            setStyle(KoListStyle::Malayalam);
            break;
        case KoOdfNumberDefinition::Kannada:
            setStyle(KoListStyle::Kannada);
            break;
        case KoOdfNumberDefinition::Gurumukhi:
            setStyle(KoListStyle::Gurumukhi);
            break;
        case KoOdfNumberDefinition::Gujarati:
            setStyle(KoListStyle::Gujarati);
            break;
        case KoOdfNumberDefinition::Bengali:
            setStyle(KoListStyle::Bengali);
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
//         debugText << "stylename else:" << style.localName() << "level=" << level << "displayLevel=" << displayLevel;
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
            QString mode(property.attributeNS(KoXmlNS::text, "list-level-position-and-space-mode"));
            if (mode == "label-alignment") {
                QString textAlign(property.attributeNS(KoXmlNS::fo, "text-align"));
                setAlignment(textAlign.isEmpty() ? Qt::AlignLeft : KoText::alignmentFromString(textAlign));

                KoXmlElement p;
                forEachElement(p, property) {
                     if (p.namespaceURI() == KoXmlNS::style && p.localName() == "list-level-label-alignment") {
                        // The <style:list-level-label-alignment> element and the fo:text-align attribute are used to define
                        // the position and spacing of the list label and the list item. The values of the attributes for
                        // text:space-before, text:min-label-width and text:min-label-distance are assumed to be 0.
                        setAlignmentMode(true);

                        QString textindent(p.attributeNS(KoXmlNS::fo, "text-indent"));
                        QString marginleft(p.attributeNS(KoXmlNS::fo, "margin-left"));
                        qreal ti = textindent.isEmpty() ? 0 : KoUnit::parseValue(textindent);
                        qreal ml = marginleft.isEmpty() ? 0 : KoUnit::parseValue(marginleft);
                        setTextIndent(ti);
                        setMargin(ml);

                        QString labelFollowedBy(p.attributeNS(KoXmlNS::text, "label-followed-by","space"));
                        if(labelFollowedBy.compare("listtab",Qt::CaseInsensitive)==0) {

                            setLabelFollowedBy(KoListStyle::ListTab);

                            // list tab position is evaluated only if label is followed by listtab
                            // the it is only evaluated if there is a list-tab-stop-position specified
                            // if not specified use the fo:margin-left:
                            QString tabStop(p.attributeNS(KoXmlNS::text, "list-tab-stop-position"));
                            if (!tabStop.isEmpty()) {
                                qreal tabStopPos = KoUnit::parseValue(tabStop);
                                setTabStopPosition(qMax<qreal>(0.0, tabStopPos));
                            }

                        }else if(labelFollowedBy.compare("nothing",Qt::CaseInsensitive)==0) {

                            setLabelFollowedBy(KoListStyle::Nothing);

                        }else {

                            setLabelFollowedBy(KoListStyle::Space);

                        }

                        setMinimumWidth(0);
                        setMinimumDistance(0);

                        //TODO support ODF 18.829 text:label-followed-by and 18.832 text:list-tab-stop-position
                     }
                }
            }

            if(alignmentMode()!=true ){ // default is mode == "label-width-and-position"
                // The text:space-before, text:min-label-width, text:minimum-label-distance and fo:text-align attributes
                // are used to define the position and spacing of the list label and the list item.

                setAlignmentMode(false);

                QString spaceBefore(property.attributeNS(KoXmlNS::text, "space-before"));
                if (!spaceBefore.isEmpty())
                    setIndent(KoUnit::parseValue(spaceBefore));

                QString minLableWidth(property.attributeNS(KoXmlNS::text, "min-label-width"));
                if (!minLableWidth.isEmpty())
                    setMinimumWidth(KoUnit::parseValue(minLableWidth));

                QString textAlign(property.attributeNS(KoXmlNS::fo, "text-align"));
                if (!textAlign.isEmpty())
                    setAlignment(KoText::alignmentFromString(textAlign));

                QString minLableDistance(property.attributeNS(KoXmlNS::text, "min-label-distance"));
                if (!minLableDistance.isEmpty())
                    setMinimumDistance(KoUnit::parseValue(minLableDistance));               
            }

            QString width(property.attributeNS(KoXmlNS::fo, "width"));
            if (!width.isEmpty())
                setWidth(KoUnit::parseValue(width));

            QString height(property.attributeNS(KoXmlNS::fo, "height"));
            if (!height.isEmpty())
                setHeight(KoUnit::parseValue(height));

        } else if (localName == "text-properties") {
            QSharedPointer<KoCharacterStyle> charStyle(new KoCharacterStyle);
            charStyle->loadOdf(&style, scontext);
            setCharacterProperties(charStyle);
        }
    }
}

void KoListLevelProperties::saveOdf(KoXmlWriter *writer, KoShapeSavingContext &context) const
{
    bool isNumber = KoListStyle::isNumberingStyle(d->stylesPrivate.value(QTextListFormat::ListStyle).toInt());

    if (isNumber || isOutlineList()) {
        if (isOutlineList()) {
            writer->startElement("text:outline-level-style"); } else {
            writer->startElement("text:list-level-style-number");
        }

        if (d->stylesPrivate.contains(KoListStyle::StartValue))
            writer->addAttribute("text:start-value", d->stylesPrivate.value(KoListStyle::StartValue).toInt());
        if (d->stylesPrivate.contains(KoListStyle::DisplayLevel))
            writer->addAttribute("text:display-levels", d->stylesPrivate.value(KoListStyle::DisplayLevel).toInt());

        QByteArray format;
        switch (style()) {
        case KoListStyle::DecimalItem:      format = "1"; break;
        case KoListStyle::AlphaLowerItem:   format = "a"; break;
        case KoListStyle::UpperAlphaItem:   format = "A"; break;
        case KoListStyle::RomanLowerItem:   format = "i"; break;
        case KoListStyle::UpperRomanItem:   format = "I"; break;
        case KoListStyle::ArabicAlphabet:   format = "أ, ب, ت, ..."; break;
        case KoListStyle::Thai:             format = "ก, ข, ค, ..."; break;
        case KoListStyle::Abjad:            format = "أ, ب, ج, ..."; break;
        case KoListStyle::AbjadMinor:       format = "ﺃ,ﺏ, ﺝ, ... "; break;
        case KoListStyle::Telugu:           format = "౧, ౨, ౩, ..."; break;
        case KoListStyle::Tamil:            format = "௧, ௨, ௪, ..."; break;
        case KoListStyle::Oriya:            format = "୧, ୨, ୩, ..."; break;
        case KoListStyle::Malayalam:        format = "൧, ൨, ൩, ..."; break;
        case KoListStyle::Kannada:          format = "೧, ೨, ೩, ..."; break;
        case KoListStyle::Gurumukhi:        format = "੧, ੨, ੩, ..."; break;
        case KoListStyle::Gujarati:         format = "૧, ૨, ૩, ..."; break;
        case KoListStyle::Bengali:          format = "১, ২, ৩, ..."; break;
        default:                            format = ""; break;
        }
        writer->addAttribute("style:num-format", format);
    }
    else if (style() == KoListStyle::ImageItem) {
        KoImageData *imageData = d->stylesPrivate.value(KoListStyle::BulletImage).value<KoImageData *>();
        if (imageData && imageData->priv()->collection) {
            writer->startElement("text:list-level-style-image");
            writer->addAttribute("xlink:show", "embed");
            writer->addAttribute("xlink:actuate", "onLoad");
            writer->addAttribute("xlink:type", "simple");
            writer->addAttribute("xlink:href", context.imageHref(imageData));
            context.addDataCenter(imageData->priv()->collection);
        }
    }
    else {
        writer->startElement("text:list-level-style-bullet");

        int bullet;
        if (d->stylesPrivate.contains(KoListStyle::BulletCharacter)) {
            bullet = d->stylesPrivate.value(KoListStyle::BulletCharacter).toInt();
        } else { // try to determine the bullet character from the style
            bullet = KoListStyle::bulletCharacter(style());
        }
        writer->addAttribute("text:bullet-char", QChar(bullet));
    }

    KoTextSharedSavingData *sharedSavingData = 0;
    if (d->stylesPrivate.contains(KoListStyle::CharacterStyleId) && (characterStyleId() != 0) &&
           (sharedSavingData = static_cast<KoTextSharedSavingData *>(context.sharedData(KOTEXT_SHARED_SAVING_ID)))) {
        QString styleName = sharedSavingData->styleName(characterStyleId());
               // dynamic_cast<KoTextSharedSavingData *>(context.sharedData(KOTEXT_SHARED_SAVING_ID))->styleName(characterStyleId());
        if (!styleName.isEmpty()) {
            writer->addAttribute("text:style-name", styleName);
         }
    }

    // These apply to bulleted and numbered lists
    if (d->stylesPrivate.contains(KoListStyle::Level))
        writer->addAttribute("text:level", d->stylesPrivate.value(KoListStyle::Level).toInt());
    if (d->stylesPrivate.contains(KoListStyle::ListItemPrefix))
        writer->addAttribute("style:num-prefix", d->stylesPrivate.value(KoListStyle::ListItemPrefix).toString());
    if (d->stylesPrivate.contains(KoListStyle::ListItemSuffix))
        writer->addAttribute("style:num-suffix", d->stylesPrivate.value(KoListStyle::ListItemSuffix).toString());

    writer->startElement("style:list-level-properties", false);

    if (d->stylesPrivate.contains(KoListStyle::Width)) {
        writer->addAttribute("fo:width", width());
    }
    if (d->stylesPrivate.contains(KoListStyle::Height)) {
        writer->addAttribute("fo:height", height());
    }

    if(d->stylesPrivate.contains(KoListStyle::AlignmentMode) && alignmentMode()==false) {

        writer->addAttribute("text:list-level-position-and-space-mode","label-width-and-position");

        if (d->stylesPrivate.contains(KoListStyle::Indent))
            writer->addAttribute("text:space-before", indent());

        if (d->stylesPrivate.contains(KoListStyle::MinimumWidth))
            writer->addAttribute("text:min-label-width", minimumWidth());

        if (d->stylesPrivate.contains(KoListStyle::Alignment))
            writer->addAttribute("fo:text-align", KoText::alignmentToString(alignment()));

        if (d->stylesPrivate.contains(KoListStyle::MinimumDistance))
            writer->addAttribute("text:min-label-distance", minimumDistance());
    } else {
        writer->addAttribute("text:list-level-position-and-space-mode","label-alignment");

        if (d->stylesPrivate.contains(KoListStyle::Alignment))
            writer->addAttribute("fo:text-align", KoText::alignmentToString(alignment()));

        writer->startElement("style:list-level-label-alignment");

        if(labelFollowedBy()==KoListStyle::ListTab) {
            writer->addAttribute("text:label-followed-by","listtab");
            writer->addAttribute("text:list-tab-stop-position", tabStopPosition());
        } else if (labelFollowedBy()==KoListStyle::Nothing){
            writer->addAttribute("text:label-followed-by","nothing");
        }else{
            writer->addAttribute("text:label-followed-by","space");
        }

        writer->addAttribute("fo:text-indent", textIndent());
        writer->addAttribute("fo:margin-left", margin());

        writer->endElement();
    }

    writer->endElement(); // list-level-properties

    // text properties

    if (d->stylesPrivate.contains(KoListStyle::CharacterProperties)) {
        KoGenStyle liststyle(KoGenStyle::ListStyle);

        QSharedPointer<KoCharacterStyle> cs = characterProperties();
        cs->saveOdf(liststyle);

        liststyle.writeStyleProperties(writer, KoGenStyle::TextType);
    }

//   debugText << "Key KoListStyle::ListItemPrefix :" << d->stylesPrivate.value(KoListStyle::ListItemPrefix);
//   debugText << "Key KoListStyle::ListItemSuffix :" << d->stylesPrivate.value(KoListStyle::ListItemSuffix);
//   debugText << "Key KoListStyle::CharacterStyleId :" << d->stylesPrivate.value(KoListStyle::CharacterStyleId);
//   debugText << "Key KoListStyle::RelativeBulletSize :" << d->stylesPrivate.value(KoListStyle::RelativeBulletSize);
//   debugText << "Key KoListStyle::Alignment :" << d->stylesPrivate.value(KoListStyle::Alignment);
//   debugText << "Key KoListStyle::LetterSynchronization :" << d->stylesPrivate.value(KoListStyle::LetterSynchronization);

    writer->endElement();
}
