/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGSHADESLIDER_H
#define WGSHADESLIDER_H

#include "WGSelectorWidgetBase.h"

#include <KisVisualColorModel.h>

#include <QWidget>
#include <QScopedPointer>
#include <QVector4D>

class QImage;

class WGShadeSlider : public QWidget
{
    Q_OBJECT
public:
    explicit WGShadeSlider(WGSelectorDisplayConfigSP config, QWidget *parent = nullptr, KisVisualColorModelSP model = nullptr);
    ~WGShadeSlider() override;

    void setGradient(const QVector4D &range, const QVector4D &offset);
    void setDisplayMode(bool slider, int numPatches = -1);
    void setModel(KisVisualColorModelSP model);
    QVector4D channelValues() const;
    const QImage* background();

public Q_SLOTS:
    void slotSetChannelValues(const QVector4D &values);
    void resetHandle();

protected Q_SLOTS:
    void slotDisplayConfigurationChanged();

protected:
    QSize minimumSizeHint() const override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    //void tabletEvent(QTabletEvent *event) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent *) override;

    bool adjustHandleValue(const QPointF &widgetPos);
    QPointF convertSliderValueToWidgetCoordinate(qreal value);
    qreal convertWidgetCoordinateToSliderValue(QPointF coordinate);
    QVector4D calculateChannelValues(qreal sliderPos) const;
    int getPatch(const QPointF pos) const;
    QRectF patchRect(int index) const;
    void recalculateParameters();
    bool sizeRequirementsMet() const;
    QImage renderBackground();
    /*!
     * \brief strokeRect
     * \param painter shall already be scaled so that 1 unit == 1 real Pixel
     * \param rect the rectangle to stroke in (logical) widget coordinates
     * \param pixelSize devicePixelRatioF() that was used to determine the real dimensions
     * \param shrinkX shrinks the rect by a multiple of the line width used to stroke
     */
    void strokeRect(QPainter &painter, const QRectF &rect, qreal pixelSize, qreal shrinkX);

Q_SIGNALS:
    void sigChannelValuesChanged(const QVector4D &values);
    void sigInteraction(bool active);
private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // WGSHADESLIDER_H
