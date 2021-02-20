/*
 * SPDX-FileCopyrightText: 2014 Manuel Riecke <spell1337@gmail.com>
 *
 * SPDX-License-Identifier: ICS
 */

#ifndef KISWDGINDEXCOLORS_H
#define KISWDGINDEXCOLORS_H

#include <kis_config_widget.h>
#include <QSpinBox>

class QCheckBox;
class KisColorButton;
namespace Ui
{
class KisWdgIndexColors;
}

class KisWdgIndexColors : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgIndexColors(QWidget* parent = 0, Qt::WindowFlags f = Qt::WindowFlags(), int delay = 500);
    KisPropertiesConfigurationSP configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;
    void setup(QStringList shadesLabels, int ramps);

private Q_SLOTS:
    void slotColorLimitChanged(int value);

private:
    struct ColorWidgets
    {
        KisColorButton* button;
        QCheckBox* checkbox;
    };
    QVector< QVector<ColorWidgets> > m_colorSelectors;
    QVector< QSpinBox* > m_stepSpinners;
    Ui::KisWdgIndexColors* ui;
};

#endif // KISWDGINDEXCOLORS_H
