/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef SVGTEXTLABEL_H
#define SVGTEXTLABEL_H

#include <QQuickPaintedItem>
#include <QFont>
#include <QScopedPointer>

/**
 * SvgTextLabel makes it possible to paint a KoSvgTextShape as a QML item.
 * It will position and scale the text centered in the bounds of the items.
 * 
 * This is nowhere near as fast as a regular Text item, as Qt has a glyph
 * cache, while Krita's system paints paths directly.
 */

class SvgTextLabel : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QStringList fontFamilies READ fontFamilies WRITE setFontFamilies NOTIFY fontFamiliesChanged)
    Q_PROPERTY(qreal fontWeight READ fontWeight WRITE setFontWeight NOTIFY fontWeightChanged)
    Q_PROPERTY(qreal fontWidth READ fontWidth WRITE setFontWidth NOTIFY fontWidthChanged)
    Q_PROPERTY(QFont::Style fontStyle READ fontStyle WRITE setFontStyle NOTIFY fontStyleChanged)
    Q_PROPERTY(qreal fontSlant READ fontSlant WRITE setFontSlant NOTIFY fontSlantChanged)
    Q_PROPERTY(QVariantMap fontAxesValues READ fontAxesValues WRITE setFontAxesValues NOTIFY fontAxesValuesChanged)
    Q_PROPERTY(qreal fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QStringList openTypeFeatures READ openTypeFeatures WRITE setOpenTypeFeatures NOTIFY openTypeFeaturesChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)

public:
    SvgTextLabel(QQuickItem *parent = nullptr);
    ~SvgTextLabel();

    void paint(QPainter *painter) override;

    QStringList fontFamilies() const;

    qreal fontWeight() const;

    qreal fontWidth() const;

    QFont::Style fontStyle() const;

    qreal fontSlant() const;

    QVariantMap fontAxesValues() const;

    qreal fontSize() const;

    QColor textColor() const;

    QStringList openTypeFeatures() const;

    QString text() const;

    int padding() const;

    QString language() const;

public Q_SLOTS:
    void setFontFamilies(QStringList fontFamilies);

    void setFontWeight(qreal fontWeight);

    void setFontWidth(qreal fontWidth);

    void setFontStyle(QFont::Style fontStyle);

    void setFontSlant(qreal fontSlant);

    void setFontAxesValues(QVariantMap fontAxesValues);

    void setFontSize(qreal fontSize);

    void setTextColor(QColor textColor);

    void setOpenTypeFeatures(QStringList openTypeFeatures);

    void setText(QString text);

    void setPadding(int padding);

    void setLanguage(QString language);

Q_SIGNALS:
    void fontFamiliesChanged(QStringList);

    void fontWeightChanged(qreal fontWeight);

    void fontWidthChanged(qreal fontWidth);

    void fontStyleChanged(QFont::Style fontStyle);

    void fontSlantChanged(qreal fontSlant);

    void fontAxesValuesChanged(QVariantMap fontAxesValues);

    void fontSizeChanged(qreal fontSize);

    void textColorChanged(QColor textColor);

    void openTypeFeaturesChanged(QStringList openTypeFeatures);

    void textChanged(QString text);

    void paddingChanged(int padding);

    void languageChanged(QString language);
protected:
    /**
     * @brief componentComplete
     * called when all properties have been set.
     * used to avoid relayout being called time and time again when the properties change.
     */
    void componentComplete() override;

private:

    void updateShape();

    struct Private;
    const QScopedPointer<Private> d;
};

#endif // SVGTEXTLABEL_H
