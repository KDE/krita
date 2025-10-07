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
 * A QQuickPaintedItem that will load SVG as KoShapes
 * and paint them.
 */
class KoShapeQtQuickLabel: public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString svgData READ svgData WRITE setSvgData NOTIFY svgDataChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY foregroundColorChanged)
    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)
    Q_PROPERTY(bool fullColor READ fullColor WRITE setFullColor NOTIFY fullColorChanged)
    Q_PROPERTY(ScalingType scalingType READ scalingType WRITE setScalingType NOTIFY scalingTypeChanged)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)
    Q_PROPERTY(QRectF documentRect READ documentRect WRITE setDocumentRect NOTIFY documentRectChanged)
    Q_PROPERTY(QRectF minimumRect READ minimumRect NOTIFY minimumRectChanged)

    QML_NAMED_ELEMENT(KoShapeQtQuickLabel)
public:
    KoShapeQtQuickLabel(QQuickItem *parent = nullptr);
    ~KoShapeQtQuickLabel();

    enum ScalingType {
        Fit, ///< Default koshapepainter behaviour, fit whole document into widget bounds.
        FitWidth, ///< Scale document view to document width. Use alignment to position top or bottom on the document.
        FitHeight ///< Scale document view to document height. Use alignment to position left or right on the document.
    };
    Q_ENUM(ScalingType)

    // QQuickPaintedItem interface
public:
    void paint(QPainter *painter) override;
    QString svgData() const;
    void setSvgData(const QString &newSvgData);

    /**
     * @brief foregroundColor
     * This is the color that is used as the fill for the KoShapes when fullColor is set false.
     */
    QColor foregroundColor() const;
    void setForegroundColor(const QColor &newForegroundColor);

    /**
     *  Padding of the documentView to the borders.
     *  This will add extra padding to the sides. If there's still paintable
     *  shape data in that padding, it will be drawn as well.
     */
    int padding() const;
    void setPadding(int newPadding);

    /**
     * @brief fullColor
     * This controls whether the current shape background will be set to the foreground color
     * or whether it will keep the color it received during parsing of SVG data.
     */
    bool fullColor() const;
    void setFullColor(bool newFullColor);

    /**
     * @brief scalingType
     * @see ScalingType enum.
     */
    ScalingType scalingType() const;
    void setScalingType(const ScalingType type);

    /**
     * @brief alignment
     * @return how to align the document viewport when using FillWidth or FillHeight as the scaling type.
     */
    Qt::Alignment alignment() const;
    void setAlignment(const Qt::Alignment align);

    /**
     * By default, this label will use the content rect as the document view.
     * When this rectangle is valid, it will be used instead.
     **/
    QRectF documentRect() const;
    void setDocumentRect(const QRectF &rect);

    /**
     * @brief minimumRect
     * Minimum size rectangle, defined by either documentRect or loaded SVG contentRect,
     * with padding attached. Use this when you want sensible minimum size.
     * We cannot set this to implicitsize or the like, because implicit width and height
     * need to be used when setting KoShapeQtQuickLabel as a content item to a QQuickControl.
     */
    QRectF minimumRect() const;

    void componentComplete() override;

Q_SIGNALS:
    void svgDataChanged();

    void fillColorChanged();

    void foregroundColorChanged();

    void paddingChanged();

    void fullColorChanged();

    void scalingTypeChanged();

    void alignmentChanged();

    void documentRectChanged();

    void minimumRectChanged();

private Q_SLOTS:
    void updateShapes();

    void callUpdateIfComplete();

private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif // KOSHAPEQTQUICKLABEL_H
