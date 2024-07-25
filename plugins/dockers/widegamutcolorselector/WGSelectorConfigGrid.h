/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGSELECTORCONFIGGRID_H
#define WGSELECTORCONFIGGRID_H

#include <KisVisualColorModel.h>
#include <KisColorSelectorConfiguration.h>

#include <QWidget>
#include <QVector>
#include <QIcon>

class QGridLayout;
class QActionGroup;
class KisVisualColorSelector;

class WGSelectorConfigGrid : public QWidget
{
    Q_OBJECT
public:
    explicit WGSelectorConfigGrid(QWidget *parent = nullptr, bool multiSelect = false);

    void clear();
    QIcon currentIcon() const;
    KisColorSelectorConfiguration currentConfiguration() const;
    QVector<KisColorSelectorConfiguration> selectedConfigurations() const;
    void setColorModel(KisVisualColorModel::ColorModel model);
    void setConfigurations(const QVector<KisColorSelectorConfiguration> &configurations);
    void setChecked(const KisColorSelectorConfiguration &configuration);
    QIcon generateIcon(const KisColorSelectorConfiguration &configuration, qreal pixelRatio = 1.0, bool dualState = false) const;

    static QVector<KisColorSelectorConfiguration> hueBasedConfigurations();
protected:
    bool event(QEvent *event) override;
private Q_SLOTS:
    void slotActionTriggered(QAction *action);
Q_SIGNALS:
    void sigConfigSelected(const KisColorSelectorConfiguration &cfg);
private:
    void updateIcons();

    int m_columns {4};
    int m_iconSize {96};
    QGridLayout *m_layout;
    QActionGroup *m_actionGroup;
    KisVisualColorSelector *m_selector;
    QAction *m_dummyAction {0};
    QAction *m_currentAction {0};
};

#endif // WGSELECTORCONFIGGRID_H
