/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_EQUALIZER_BUTTON_H
#define __KIS_EQUALIZER_BUTTON_H

#include <QScopedPointer>
#include <QAbstractButton>


class KisEqualizerButton : public QAbstractButton
{
public:
    KisEqualizerButton(QWidget *parent);
    ~KisEqualizerButton() override;

    void paintEvent(QPaintEvent *event) override;
    void setRightmost(bool value);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_EQUALIZER_BUTTON_H */
