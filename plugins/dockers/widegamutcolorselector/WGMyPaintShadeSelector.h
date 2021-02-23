/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGMYPAINTSHADESELECTOR_H
#define WGMYPAINTSHADESELECTOR_H

#include "WGSelectorWidgetBase.h"

#include <kis_types.h>

class KisDisplayColorConverter;

/**
 * @brief A port of MyPaint's "Crossed Bowl" color selector
 */

class WGMyPaintShadeSelector : public WGSelectorWidgetBase
{
    Q_OBJECT
public:
    WGMyPaintShadeSelector(QWidget *parent, UiMode mode);
    ~WGMyPaintShadeSelector() override;

    void setModel(KisVisualColorModelSP model) override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;

    bool getChannelValues(QPoint pos, QVector4D &values, QVector4D &blendValues);
    void pickColorAt(const QPointF &posF);

private Q_SLOTS:
    void slotSetChannelValues(const QVector4D &values);

private:
    KisVisualColorModelSP m_model;
    KisPaintDeviceSP m_realPixelCache;
    KisPaintDeviceSP m_realCircleBorder;
    KisDisplayColorConverter *m_converter;
    float m_colorH, m_colorS, m_colorV;
    int m_sizeHD;
    int m_widthHD;
    int m_heightHD;
    bool m_allowUpdates {true};
};

#endif // WGMYPAINTSHADESELECTOR_H
