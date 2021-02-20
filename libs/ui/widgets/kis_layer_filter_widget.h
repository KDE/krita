/*
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    bool hasTextFilter() const;
    QSet<int> getActiveColors() const;
    QString getTextFilter() const;

    int getDesiredMinimumWidth() const;
    int getDesiredMinimumHeight() const;

    void reset();

    QSize sizeHint() const override;

    /* Show Event has to be overridden to
     * correct for parent QMenu to properly
     * resize. */
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
    void setTextFilter(bool isTextFiltering);


private:
    void paintEvent(QPaintEvent *paintEvent) override;

private:
    bool m_textFilter;
    QList<int> m_selectedColors;
};

class KRITAUI_EXPORT MouseClickIgnore : public QObject {
    Q_OBJECT
public:
    MouseClickIgnore(QObject *parent = nullptr);
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // KISLAYERFILTERWIDGET_H
