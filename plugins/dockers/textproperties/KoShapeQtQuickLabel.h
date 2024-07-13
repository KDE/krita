/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOSHAPEQTQUICKLABEL_H
#define KOSHAPEQTQUICKLABEL_H

#include <QQuickPaintedItem>

/**
 * @brief The KoShapeQtQuickLabel class
 *
 * A QQuickPaintedItem that will load SVG as a KoShape
 * and paint it.
 */
class KoShapeQtQuickLabel: public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString svgData READ svgData WRITE setSvgData NOTIFY svgDataChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY foregroundColorChanged)
    Q_PROPERTY(int imagePadding READ imagePadding WRITE setImagePadding NOTIFY imagePaddingChanged)
    Q_PROPERTY(qreal imageScale READ imageScale WRITE setImageScale NOTIFY imageScaleChanged)
    Q_PROPERTY(bool fullColor READ fullColor WRITE setFullColor NOTIFY fullColorChanged)
public:
    KoShapeQtQuickLabel(QQuickItem *parent = nullptr);
    ~KoShapeQtQuickLabel();

    // QQuickPaintedItem interface
public:
    void paint(QPainter *painter) override;
    QString svgData() const;
    void setSvgData(const QString &newSvgData);

    /// Foreground color.
    QColor foregroundColor() const;
    void setForegroundColor(const QColor &newForegroundColor);

    /// Padding of the image to the borders.
    int imagePadding() const;
    void setImagePadding(int newPadding);

    /// extra scaling applied to the image.
    qreal imageScale() const;
    void setImageScale(qreal newImageScale);

    bool fullColor() const;
    void setFullColor(bool newFullColor);

Q_SIGNALS:
    void svgDataChanged();

    void fillColorChanged();

    void foregroundColorChanged();

    void imagePaddingChanged();

    void imageScaleChanged();

    void fullColorChanged();

private Q_SLOTS:
    void updateShapes();

private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif // KOSHAPEQTQUICKLABEL_H
