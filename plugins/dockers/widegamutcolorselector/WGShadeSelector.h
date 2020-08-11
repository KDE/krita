/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGSHADESELECTOR_H
#define WGSHADESELECTOR_H

#include <QObject>
#include <QWidget>
#include <QVector>
#include <QVector4D>

class KisVisualColorModel;
class WGShadeSlider;

class WGShadeSelector : public QWidget
{
    Q_OBJECT

    struct LineConfig {
        QVector4D gradient;
        QVector4D offset;
    };

public:
    explicit WGShadeSelector(KisVisualColorModel *selector, QWidget *parent = nullptr);

    static QVector<LineConfig> loadConfiguration();
    void updateSettings();
public Q_SLOTS:
    void slotChannelValuesChanged(const QVector4D &values);
private Q_SLOTS:
    void slotSliderValuesChanged(const QVector4D &values);

Q_SIGNALS:
    void sigChannelValuesChanged(const QVector4D &values);
    void sigColorInteraction(bool active);

private:
    KisVisualColorModel *m_model;
    QVector<WGShadeSlider *> m_sliders;
    int m_lineHeight {10};
    bool m_allowUpdates {true};
};

#endif // WGSHADESELECTOR_H
