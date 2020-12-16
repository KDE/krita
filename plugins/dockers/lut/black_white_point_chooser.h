/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __BLACK_WHITE_POINT_CHOOSER_H
#define __BLACK_WHITE_POINT_CHOOSER_H

#include <QFrame>

class KisDoubleSliderSpinBox;


class BlackWhitePointChooser : public QFrame
{
    Q_OBJECT
public:
    BlackWhitePointChooser(QWidget* parent);
    ~BlackWhitePointChooser();

    void showPopup(const QPoint &basePoint);

    qreal blackPoint() const;
    void setBlackPoint(qreal bp);

    qreal whitePoint() const;
    void setWhitePoint(qreal wp);

Q_SIGNALS:
    void sigBlackPointChanged(qreal value);
    void sigWhitePointChanged(qreal value);

private:
    KisDoubleSliderSpinBox *m_black;
    KisDoubleSliderSpinBox *m_white;
};

#endif /* __BLACK_WHITE_POINT_CHOOSER_H */
