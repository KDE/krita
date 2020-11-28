/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_EQUALIZER_WIDGET_H
#define __KIS_EQUALIZER_WIDGET_H

#include <QScopedPointer>
#include <QWidget>
#include <QMap>


#include "kritaanimationdocker_export.h"


class KRITAANIMATIONDOCKER_EXPORT KisEqualizerWidget : public QWidget
{
    Q_OBJECT

public:
    KisEqualizerWidget(int maxDistance, QWidget *parent);
    ~KisEqualizerWidget() override;

    struct EqualizerValues {
        int maxDistance;
        QMap<int, qreal> value;
        QMap<int, bool> state;
    };

    EqualizerValues getValues() const;
    void setValues(const EqualizerValues &values);

    void toggleMasterSwitch();

    void resizeEvent(QResizeEvent *event) override;

    void mouseMoveEvent(QMouseEvent *ev) override;

Q_SIGNALS:
    void sigConfigChanged();

private Q_SLOTS:
    void slotMasterColumnChanged(int, bool, int);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_EQUALIZER_WIDGET_H */
