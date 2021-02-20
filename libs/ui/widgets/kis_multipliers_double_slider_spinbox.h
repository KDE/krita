/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_MULTIPLIERS_DOUBLE_SLIDER_SPINBOX_H_
#define _KIS_MULTIPLIERS_DOUBLE_SLIDER_SPINBOX_H_

#include <QWidget>

#include <kritaui_export.h>


/**
 * This class add a combobox to a \ref KisDoubleSliderSpinBox which
 * allows to define a multiplier to let the user change the range.
 */
class KRITAUI_EXPORT KisMultipliersDoubleSliderSpinBox : public QWidget {
    Q_OBJECT
public:
    KisMultipliersDoubleSliderSpinBox(QWidget* _parent = 0);
    ~KisMultipliersDoubleSliderSpinBox() override;
    
    void addMultiplier(double v);
    /**
     * Set the range for the 1.0 multiplier
     */
    void setRange(qreal minimum, qreal maximum, int decimals = 0);
    
    ///Get the value, don't use value()
    qreal value();

    ///Set the value, don't use setValue()
    void setValue(qreal value);
    void setExponentRatio(qreal dbl);

    void setPrefix(const QString& prefix);
    void setSuffix(const QString& suffix);

    void setBlockUpdateSignalOnDrag(bool block);

    void setSingleStep(qreal value);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    virtual QSize minimumSize() const;

Q_SIGNALS:
    void valueChanged(qreal value);
    
private:
    Q_PRIVATE_SLOT(d, void updateRange())
    struct Private;
    Private* const d;
};


#endif
