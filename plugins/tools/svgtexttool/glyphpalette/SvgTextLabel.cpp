/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "SvgTextLabel.h"
#include <KoSvgTextShape.h>
#include <KoSvgTextProperties.h>
#include <KoColorBackground.h>
#include <QPainter>
#include <QImage>
#include <QQuickWindow>
#include <KoShapePainter.h>
#include <kis_assert.h>

struct Q_DECL_HIDDEN SvgTextLabel::Private {

    Private()
        : shape(new KoSvgTextShape())
    {
        shape->setResolution(72, 72);
        shape->setRelayoutBlocked(true);
        shape->setFontMatchingDisabled(true);
    }
    ~Private() {}

    QScopedPointer<KoSvgTextShape> shape;

    /**
     * The shape painter is initialized only after QML item loading
     * is completed
     */
    QScopedPointer<KoShapePainter> shapePainter;

    KoSvgTextProperties props;
    bool textHasChanged = false;
    int padding = 2;
};

SvgTextLabel::SvgTextLabel(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , d(new Private())
{
    setImplicitSize(10, 10);
    setOpaquePainting(true);
    setAntialiasing(true);
}

SvgTextLabel::~SvgTextLabel()
{
}

void SvgTextLabel::paint(QPainter *painter)
{
    // the QML item must be finalized before paint() is called
    KIS_SAFE_ASSERT_RECOVER_RETURN(d->shapePainter);

    if (d->shape->textProperties() != d->props) {
        updateShape();
    }
    if (!painter->isActive()) return;
    painter->save();
    painter->fillRect(0, 0, width(), height(), fillColor());
    if (d->shape->boundingRect().isEmpty()) {
        painter->restore();
        return;
    }

    QRectF bbox = d->shape->selectionBoxes(0, d->shape->posForIndex(d->shape->plainText().size())).boundingRect();
    bbox = bbox.isEmpty()? d->shape->boundingRect(): bbox;

    d->shapePainter->paint(*painter,
                           QRectF(d->padding, d->padding, width()-(d->padding*2), height()-(d->padding*2)).toAlignedRect(),
                           bbox);
    painter->restore();
}

QStringList SvgTextLabel::fontFamilies() const
{
    return d->shape->textProperties().property(KoSvgTextProperties::FontFamiliesId).value<QStringList>();
}

qreal SvgTextLabel::fontWeight() const
{
    return d->shape->textProperties().property(KoSvgTextProperties::FontWeightId).toReal();
}

qreal SvgTextLabel::fontWidth() const
{
    return d->shape->textProperties().property(KoSvgTextProperties::FontStretchId).toReal();
}

QFont::Style SvgTextLabel::fontStyle() const
{
    return d->shape->textProperties().property(KoSvgTextProperties::FontStyleId).value<KoSvgText::CssFontStyleData>().style;
}

qreal SvgTextLabel::fontSlant() const
{
    KoSvgText::CssFontStyleData style = d->shape->textProperties().property(KoSvgTextProperties::FontStyleId).value<KoSvgText::CssFontStyleData>();
    return style.slantValue.isAuto? 14.0: style.slantValue.customValue;
}

QVariantMap SvgTextLabel::fontAxesValues() const
{
    return d->props.propertyOrDefault(KoSvgTextProperties::FontVariationSettingsId).toMap();
}

qreal SvgTextLabel::fontSize() const
{
    return d->shape->textProperties().property(KoSvgTextProperties::FontSizeId).toReal();
}

QColor SvgTextLabel::textColor() const
{
    KoColorBackground *bg = dynamic_cast<KoColorBackground*>(d->shape->background().data());
    if (bg) {
        return bg->color();
    }
    return Qt::black;
}

QStringList SvgTextLabel::openTypeFeatures() const
{
    return d->shape->textProperties().property(KoSvgTextProperties::FontFeatureSettingsId).value<QStringList>();
}

QString SvgTextLabel::text() const
{
    return d->shape->plainText();
}

int SvgTextLabel::padding() const
{
    return d->padding;
}

QString SvgTextLabel::language() const
{
    return d->shape->textProperties().property(KoSvgTextProperties::TextLanguage).toString();
}

void SvgTextLabel::setFontFamilies(QStringList fontFamilies)
{
    QStringList current = d->props.property(KoSvgTextProperties::FontFamiliesId).value<QStringList>();
    if (current == fontFamilies)
        return;

    d->props.setProperty(KoSvgTextProperties::FontFamiliesId, fontFamilies);
    emit fontFamiliesChanged(fontFamilies);
}

void SvgTextLabel::setFontWeight(qreal fontWeight)
{
    qreal current = d->props.property(KoSvgTextProperties::FontWeightId).toReal();
    if (qFuzzyCompare(current, fontWeight))
        return;

    d->props.setProperty(KoSvgTextProperties::FontWeightId, fontWeight);
    emit fontWeightChanged(fontWeight);
}

void SvgTextLabel::setFontWidth(qreal fontWidth)
{
    qreal current = d->props.property(KoSvgTextProperties::FontStretchId).toReal();
    if (qFuzzyCompare(current, fontWidth))
        return;

    d->props.setProperty(KoSvgTextProperties::FontStretchId, fontWidth);
    emit fontWidthChanged(fontWidth);
}

void SvgTextLabel::setFontStyle(QFont::Style fontStyle)
{
    KoSvgText::CssFontStyleData current = d->props.property(KoSvgTextProperties::FontStyleId).value<KoSvgText::CssFontStyleData>();
    if (current.style == fontStyle)
        return;

    current.style = fontStyle;
    d->props.setProperty(KoSvgTextProperties::FontStyleId, QVariant::fromValue(current));
    emit fontStyleChanged(fontStyle);
}

void SvgTextLabel::setFontSlant(qreal fontSlant)
{
    KoSvgText::CssFontStyleData current = d->props.property(KoSvgTextProperties::FontStyleId).value<KoSvgText::CssFontStyleData>();
    if (current.slantValue.isAuto && qFuzzyCompare(fontSlant, 14))
        return;
    if (!current.slantValue.isAuto && qFuzzyCompare(fontSlant, current.slantValue.customValue))
        return;

    current.slantValue.customValue = fontSlant;
    current.slantValue.isAuto = false;
    d->props.setProperty(KoSvgTextProperties::FontStyleId, QVariant::fromValue(current));
    emit fontSlantChanged(fontSlant);
}

void SvgTextLabel::setFontAxesValues(QVariantMap fontAxesValues)
{
    QVariantMap current = d->props.property(KoSvgTextProperties::FontVariationSettingsId).toMap();
    if (current == fontAxesValues)
        return;

    d->props.setProperty(KoSvgTextProperties::FontVariationSettingsId, QVariant::fromValue(fontAxesValues));
    emit fontAxesValuesChanged(fontAxesValues);
}

void SvgTextLabel::setFontSize(qreal fontSize)
{
    KoSvgText::CssLengthPercentage current = d->props.property(KoSvgTextProperties::FontSizeId).value<KoSvgText::CssLengthPercentage>();
    if (qFuzzyCompare(current.value, fontSize))
        return;

    current.value = fontSize;
    d->props.setProperty(KoSvgTextProperties::FontSizeId, QVariant::fromValue(current));
    emit fontSizeChanged(fontSize);
}

void SvgTextLabel::setTextColor(QColor textColor)
{
    if (d->props.hasProperty(KoSvgTextProperties::FillId)) {
        KoColorBackground *bg = dynamic_cast<KoColorBackground*>(d->shape->background().data());
        if (bg) {
            if (bg->color() == textColor)
                return;
        }
    }

    d->shape->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(textColor)));
    d->props.setProperty(KoSvgTextProperties::FillId, d->shape->textProperties().property(KoSvgTextProperties::FillId));
    if (this->window()) {
        const qreal pixelRatio = this->window()->effectiveDevicePixelRatio();
        d->shape->setResolution(pixelRatio*72, pixelRatio*72);
    }
    emit textColorChanged(textColor);
}

void SvgTextLabel::setOpenTypeFeatures(QStringList openTypeFeatures)
{
    QStringList otf = d->props.property(KoSvgTextProperties::FontFeatureSettingsId).value<QStringList>();
    if (otf == openTypeFeatures)
        return;

    d->props.setProperty(KoSvgTextProperties::FontFeatureSettingsId, openTypeFeatures);
    emit openTypeFeaturesChanged(openTypeFeatures);
}

void SvgTextLabel::setText(QString text)
{
    if (d->shape->plainText() == text)
        return;

    int length = d->shape->plainText().size();
    int start = 0;
    d->shape->removeText(start, length);
    d->shape->insertText(0, text);
    emit textChanged(d->shape->plainText());
}

void SvgTextLabel::setPadding(int padding)
{
    if (d->padding == padding)
        return;

    d->padding = padding;
    emit paddingChanged(d->padding);
}

void SvgTextLabel::setLanguage(QString language)
{
    const QString lang = d->props.property(KoSvgTextProperties::TextLanguage).toString();
    if (lang == language) {
        return;
    }
    d->props.setProperty(KoSvgTextProperties::TextLanguage, language);
    emit languageChanged(language);
}

void SvgTextLabel::componentComplete()
{
    QQuickPaintedItem::componentComplete();

    updateShape();
    if (!d->shapePainter) {
        d->shapePainter.reset(new KoShapePainter());
        d->shapePainter->setShapes({d->shape.data()});
    }

}

void SvgTextLabel::updateShape()
{
    d->shape->setPropertiesAtPos(-1, d->props);
    d->shape->relayout();
    setImplicitSize(d->shape->boundingRect().width(), d->shape->boundingRect().height());
}
