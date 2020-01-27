/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2008-08-16
 * @brief  Integer and double num input widget
 *         re-implemented with a reset button to switch to
 *         a default value
 *
 * @author Copyright (C) 2008-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef RNUMINPUT_H
#define RNUMINPUT_H

// Qt includes

#include <QWidget>

// Local includes



namespace KDcrawIface
{

class  RIntNumInput : public QWidget
{
    Q_OBJECT

public:

    RIntNumInput(QWidget* const parent=0);
    ~RIntNumInput() override;

    void setRange(int min, int max, int step);

    void setDefaultValue(int d);
    int  defaultValue() const;
    int  value()        const;

    void setSuffix(const QString& suffix);

Q_SIGNALS:

    void reset();
    void valueChanged(int);

public Q_SLOTS:

    void setValue(int d);
    void slotReset();

private Q_SLOTS:

    void slotValueChanged(int);

private:

    class Private;
    Private* const d;
};

// ---------------------------------------------------------

class  RDoubleNumInput : public QWidget
{
    Q_OBJECT

public:

    RDoubleNumInput(QWidget* const parent=0);
    ~RDoubleNumInput() override;

    void   setDecimals(int p);
    void   setRange(double min, double max, double step);

    void   setDefaultValue(double d);
    double defaultValue() const;
    double value()        const;

    void setSuffix(const QString& suffix);

Q_SIGNALS:

    void reset();
    void valueChanged(double);

public Q_SLOTS:

    void setValue(double d);
    void slotReset();

private Q_SLOTS:

    void slotValueChanged(double);

private:

    class Private;
    Private* const d;
};

}  // namespace KDcrawIface

#endif /* RNUMINPUT_H */
