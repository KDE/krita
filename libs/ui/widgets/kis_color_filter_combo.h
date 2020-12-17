/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_COLOR_FILTER_COMBO_H
#define __KIS_COLOR_FILTER_COMBO_H

#include <QScopedPointer>
#include <QComboBox>
#include <QStylePainter>
#include "kritaui_export.h"
#include "kis_types.h"

class ComboEventFilter;

class KRITAUI_EXPORT KisColorFilterCombo : public QComboBox
{
    Q_OBJECT
public:
    KisColorFilterCombo(QWidget *parent, bool filterMode = true, bool circleMode = true);
    ~KisColorFilterCombo() override;

    void updateAvailableLabels(KisNodeSP rootNode);
    void updateAvailableLabels(const QSet<int> &labels);
    void setModes(bool filterMode, bool circleMode);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    QList<int> selectedColors() const;

Q_SIGNALS:
    void selectedColorsChanged();

public:
    static void paintColorPie(QStylePainter &painter, const QPalette& palette, const QList<int> &selectedColors, const QRect &rect, const int &baseSize);

private:
    void paintEvent(QPaintEvent *event) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
    QList<ComboEventFilter *> m_eventFilters;
};

#endif /* __KIS_COLOR_FILTER_COMBO_H */
