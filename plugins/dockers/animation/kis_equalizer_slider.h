/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_EQUALIZER_SLIDER_H
#define __KIS_EQUALIZER_SLIDER_H

#include <QScopedPointer>
#include <QAbstractSlider>

#include "kritaanimationdocker_export.h"


class KRITAANIMATIONDOCKER_EXPORT KisEqualizerSlider : public QAbstractSlider
{
public:
    KisEqualizerSlider(QWidget *parent);
    ~KisEqualizerSlider() override;

    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void paintEvent(QPaintEvent *event) override;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setRightmost(bool value);
    void setToggleState(bool value);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_EQUALIZER_SLIDER_H */
