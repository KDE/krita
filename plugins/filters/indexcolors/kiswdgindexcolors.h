/*
 * Copyright 2014 Manuel Riecke <spell1337@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */

#ifndef KISWDGINDEXCOLORS_H
#define KISWDGINDEXCOLORS_H

#include <kis_config_widget.h>
#include <QSpinBox>

class QCheckBox;
class KColorButton;
namespace Ui
{
class KisWdgIndexColors;
}

class KisWdgIndexColors : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgIndexColors(QWidget* parent = 0, Qt::WFlags f = 0, int delay = 500);
    virtual KisPropertiesConfiguration* configuration() const;
    virtual void setConfiguration(const KisPropertiesConfiguration* config);
    void setup(QStringList shadesLabels, int ramps);

private Q_SLOTS:
    void slotColorLimitChanged(int value);

private:
    struct ColorWidgets
    {
        KColorButton* button;
        QCheckBox* checkbox;
    };
    QVector< QVector<ColorWidgets> > m_colorSelectors;
    QVector< QSpinBox* > m_stepSpinners;
    Ui::KisWdgIndexColors* ui;
};

#endif // KISWDGINDEXCOLORS_H
