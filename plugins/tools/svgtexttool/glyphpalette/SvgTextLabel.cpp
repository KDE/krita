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

struct Q_DECL_HIDDEN SvgTextLabel::Private {

    Private()
        : shape(new KoSvgTextShape())
        , shapePainter(new KoShapePainter())
    {
        shape->setResolution(72, 72);
        // Hack to avoid the textshape having 0 bounds and the KoRTree complaining about it.
        shape->insertText(0, "a");
        shape->relayout();
        shapePainter->setShapes({shape.data()});
    }
    ~Private() {}

    QScopedPointer<KoSvgTextShape> shape;
    QScopedPointer<KoShapePainter> shapePainter;
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

int SvgTextLabel::fontWeight() const
{
    return d->shape->textProperties().property(KoSvgTextProperties::FontWeightId).toInt();
}

QFont::Style SvgTextLabel::fontStyle() const
{
    return d->shape->textProperties().property(KoSvgTextProperties::FontStyleId).value<QFont::Style>();
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

void SvgTextLabel::setFontFamilies(QStringList fontFamilies)
{
    KoSvgTextProperties props = d->shape->textProperties();
    QStringList current = props.property(KoSvgTextProperties::FontFamiliesId).value<QStringList>();
    if (current == fontFamilies)
        return;

    props.setProperty(KoSvgTextProperties::FontFamiliesId, fontFamilies);
    d->shape->setPropertiesAtPos(-1, props);
    emit fontFamiliesChanged(fontFamilies);
}

void SvgTextLabel::setFontWeight(int fontWeight)
{
    KoSvgTextProperties props = d->shape->textProperties();
    int current = props.property(KoSvgTextProperties::FontWeightId).toInt();
    if (current == fontWeight)
        return;

    props.setProperty(KoSvgTextProperties::FontWeightId, fontWeight);
    d->shape->setPropertiesAtPos(-1, props);
    emit fontWeightChanged(fontWeight);
}

void SvgTextLabel::setFontStyle(QFont::Style fontStyle)
{
    KoSvgTextProperties props = d->shape->textProperties();
    QFont::Style current = props.property(KoSvgTextProperties::FontStyleId).value<QFont::Style>();
    if (current == fontStyle)
        return;

    props.setProperty(KoSvgTextProperties::FontStyleId, fontStyle);
    d->shape->setPropertiesAtPos(-1, props);
    updateShape();
    emit fontStyleChanged(fontStyle);
}

void SvgTextLabel::setFontSize(qreal fontSize)
{
    KoSvgTextProperties props = d->shape->textProperties();
    qreal current = props.property(KoSvgTextProperties::FontSizeId).toReal();
    if (qFuzzyCompare(current, fontSize))
        return;

    props.setProperty(KoSvgTextProperties::FontSizeId, fontSize);
    d->shape->setPropertiesAtPos(-1, props);
    emit fontSizeChanged(fontSize);
}

void SvgTextLabel::setTextColor(QColor textColor)
{
    KoSvgTextProperties props = d->shape->textProperties();
    if (props.hasProperty(KoSvgTextProperties::FillId)) {
        KoColorBackground *bg = dynamic_cast<KoColorBackground*>(d->shape->background().data());
        if (bg) {
            if (bg->color() == textColor)
                return;
        }
    }

    d->shape->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(textColor)));
    if (this->window()) {
        const qreal pixelRatio = this->window()->effectiveDevicePixelRatio();
        d->shape->setResolution(pixelRatio*72, pixelRatio*72);
    }
    emit textColorChanged(textColor);
}

void SvgTextLabel::setOpenTypeFeatures(QStringList openTypeFeatures)
{
    KoSvgTextProperties props = d->shape->textProperties();
    QStringList otf = props.property(KoSvgTextProperties::FontFeatureSettingsId).value<QStringList>();
    if (otf == openTypeFeatures)
        return;

    props.setProperty(KoSvgTextProperties::FontFeatureSettingsId, openTypeFeatures);
    d->shape->setPropertiesAtPos(-1, props);
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
    updateShape();
    emit textChanged(d->shape->plainText());
}

void SvgTextLabel::setPadding(int padding)
{
    if (d->padding == padding)
        return;

    d->padding = padding;
    emit paddingChanged(d->padding);
}

void SvgTextLabel::updateShape()
{
    d->shape->relayout();
    setImplicitSize(d->shape->boundingRect().width(), d->shape->boundingRect().height());
    update();
}
