/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_COLOR_FILTER_COMBO_H
#define __KIS_COLOR_FILTER_COMBO_H

#include <QScopedPointer>
#include <QComboBox>
#include "kritaui_export.h"
#include "kis_types.h"

class ComboEventFilter;

class KRITAUI_EXPORT KisColorFilterCombo : public QComboBox
{
    Q_OBJECT
public:
    KisColorFilterCombo(QWidget *parent);
    ~KisColorFilterCombo() override;

    void updateAvailableLabels(KisNodeSP rootNode);
    void updateAvailableLabels(const QSet<int> &labels);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    QList<int> selectedColors() const;

Q_SIGNALS:
    void selectedColorsChanged();

private:
    void paintEvent(QPaintEvent *event) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
    QList<ComboEventFilter *> m_eventFilters;
};

#endif /* __KIS_COLOR_FILTER_COMBO_H */
