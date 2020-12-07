/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_ANIMATION_CURVES_VALUE_RULER_H
#define _KIS_ANIMATION_CURVES_VALUE_RULER_H

#include <QHeaderView>

class KisAnimationCurvesValueRuler : public QHeaderView
{
    Q_OBJECT

public:
    KisAnimationCurvesValueRuler(QWidget *parent);
    ~KisAnimationCurvesValueRuler() override;

    void setScale(float scale);
    float scaleFactor() const;

    void setOffset(float offset);
    float offset() const;

    float mapValueToView(float value) const;
    float mapViewToValue(float y) const;

    QSize sizeHint() const override;
    void paintEvent(QPaintEvent *e) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
