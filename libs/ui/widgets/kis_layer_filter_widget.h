/*
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
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
#ifndef KISLAYERFILTERWIDGET_H
#define KISLAYERFILTERWIDGET_H

#include <QWidget>
#include <QToolButton>
#include "kis_types.h"

#include "kritaui_export.h"


class KRITAUI_EXPORT KisLayerFilterWidget : public QWidget
{
    Q_OBJECT
private:
    class KisColorLabelMouseDragFilter *buttonEventFilter;
    class QLineEdit *textFilter;
    class KisColorLabelFilterGroup *buttonGroup;
    class QPushButton *resetButton;

public:
    KisLayerFilterWidget(QWidget *parent = nullptr);

    static void scanUsedColorLabels(KisNodeSP node, QSet<int> &colorLabels);
    void updateColorLabels(KisNodeSP root);

    bool isCurrentlyFiltering() const;
    QSet<int> getActiveColors() const;
    QString getTextFilter() const;

    int getDesiredMinimumWidth() const;
    int getDesiredMinimumHeight() const;

    void reset();

    QSize sizeHint() const override;

    /* Show Event has to be overridden to
     * correct for issues where QMenu isn't
     * correctly resizing. */
    void showEvent(QShowEvent *show) override;

Q_SIGNALS:
    void filteringOptionsChanged();

};

class KRITAUI_EXPORT KisLayerFilterWidgetToolButton : public QToolButton
{
    Q_OBJECT
public:
    explicit KisLayerFilterWidgetToolButton(QWidget *parent = nullptr);
    KisLayerFilterWidgetToolButton(const KisLayerFilterWidgetToolButton& rhs);
    ~KisLayerFilterWidgetToolButton(){}

    void setSelectedColors(QList<int> colors);


private:
    void paintEvent(QPaintEvent *paintEvent) override;

private:
    bool m_textFilter;
    QList<int> m_selectedColors;
};

class KRITAUI_EXPORT MouseClickIgnore : public QWidget {
    Q_OBJECT
public:
    MouseClickIgnore(QWidget *parent = nullptr);
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // KISLAYERFILTERWIDGET_H
