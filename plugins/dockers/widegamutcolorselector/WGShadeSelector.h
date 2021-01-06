/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGSHADESELECTOR_H
#define WGSHADESELECTOR_H

#include <KisVisualColorModel.h>

#include <QObject>
#include <QWidget>
#include <QVector>
#include <QVector4D>

class WGShadeSlider;

class WGShadeSelector : public QWidget
{
    Q_OBJECT

public:
    explicit WGShadeSelector(KisVisualColorModelSP selector, QWidget *parent = nullptr);

    void updateSettings();
protected:
    void mousePressEvent(QMouseEvent *event) override;

public Q_SLOTS:
    void slotChannelValuesChanged(const QVector4D &values);
private Q_SLOTS:
    void slotSliderValuesChanged(const QVector4D &values);
    void slotSliderInteraction(bool active);
    void slotReset();

Q_SIGNALS:
    void sigChannelValuesChanged(const QVector4D &values);
    void sigColorInteraction(bool active);

private:
    KisVisualColorModelSP m_model;
    QVector<WGShadeSlider *> m_sliders;
    int m_lineHeight {10};
    bool m_resetOnExternalUpdate {true};
    bool m_resetOnInteractions {false};
    bool m_resetOnRightClick {true};
    bool m_allowUpdates {true};
    bool m_initialized {false};
};

#endif // WGSHADESELECTOR_H
