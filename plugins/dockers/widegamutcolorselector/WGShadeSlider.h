/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGSHADESLIDER_H
#define WGSHADESLIDER_H

#include <QWidget>
#include <QScopedPointer>
#include <QVector4D>

class KisVisualColorModel;

class WGShadeSlider : public QWidget
{
    Q_OBJECT
public:
    explicit WGShadeSlider(QWidget *parent = nullptr, KisVisualColorModel *model = nullptr);
    ~WGShadeSlider() override;

    void setGradient(const QVector4D &gradient);
    void setDisplayMode(bool slider, int numPatches = -1);
    QVector4D channelValues() const;

public Q_SLOTS:
    void slotSetChannelValues(const QVector4D &values);
    void resetHandle();

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
    QImage renderBackground();

Q_SIGNALS:
    void sigChannelValuesChanged(const QVector4D &values);
    void sigInteraction(bool active);
private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // WGSHADESLIDER_H
