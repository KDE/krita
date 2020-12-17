/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_EQUALIZER_COLUMN_H
#define __KIS_EQUALIZER_COLUMN_H

#include <QWidget>
#include <QScopedPointer>
#include <QSlider>

#include "kritaanimationdocker_export.h"


class KRITAANIMATIONDOCKER_EXPORT KisEqualizerColumn : public QWidget
{
    Q_OBJECT

public:
    KisEqualizerColumn(QWidget *parent, int id, const QString &title);
    ~KisEqualizerColumn() override;

    void setRightmost(bool value);

    int value() const;
    void setValue(int value);

    bool state() const;
    void setState(bool value);

    void setForceDisabled(bool value);

Q_SIGNALS:
    void sigColumnChanged(int id, bool state, int value);

private Q_SLOTS:
    void slotSliderChanged(int value);
    void slotButtonChanged(bool value);

private:
    struct Private;
    const QScopedPointer<Private> m_d;

    void updateState();
};

#endif /* __KIS_EQUALIZER_COLUMN_H */
