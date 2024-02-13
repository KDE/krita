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
    Q_PROPERTY(int fontWeight READ fontWeight WRITE setFontWeight NOTIFY fontWeightChanged)
    Q_PROPERTY(QFont::Style fontStyle READ fontStyle WRITE setFontStyle NOTIFY fontStyleChanged)
    Q_PROPERTY(qreal fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QStringList openTypeFeatures READ openTypeFeatures WRITE setOpenTypeFeatures NOTIFY openTypeFeaturesChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)

public:
    SvgTextLabel(QQuickItem *parent = nullptr);
    ~SvgTextLabel();

    void paint(QPainter *painter) override;

    QStringList fontFamilies() const;

    int fontWeight() const;

    QFont::Style fontStyle() const;

    qreal fontSize() const;

    QColor textColor() const;

    QStringList openTypeFeatures() const;

    QString text() const;

    int padding() const;

public Q_SLOTS:
    void setFontFamilies(QStringList fontFamilies);

    void setFontWeight(int fontWeight);

    void setFontStyle(QFont::Style fontStyle);

    void setFontSize(qreal fontSize);

    void setTextColor(QColor textColor);

    void setOpenTypeFeatures(QStringList openTypeFeatures);

    void setText(QString text);

    void setPadding(int padding);

Q_SIGNALS:
    void fontFamiliesChanged(QStringList);

    void fontWeightChanged(int fontWeight);

    void fontStyleChanged(QFont::Style fontStyle);

    void fontSizeChanged(qreal fontSize);

    void textColorChanged(QColor textColor);

    void openTypeFeaturesChanged(QStringList openTypeFeatures);

    void textChanged(QString text);

    void paddingChanged(int padding);

private:

    void updateShape();

    struct Private;
    const QScopedPointer<Private> d;
};

#endif // SVGTEXTLABEL_H
