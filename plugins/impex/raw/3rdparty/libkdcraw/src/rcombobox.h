/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2008-08-16
 * @brief  a combo box widget re-implemented with a
 *         reset button to switch to a default item
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

#ifndef RCOMBOBOX_H
#define RCOMBOBOX_H

// Qt includes

#include <QWidget>
#include <QComboBox>

// Local includes



namespace KDcrawIface
{

class  RComboBox : public QWidget
{

    Q_OBJECT

public:

    RComboBox(QWidget* const parent=0);
    ~RComboBox() override;

    void setCurrentIndex(int d);
    int  currentIndex() const;

    void setDefaultIndex(int d);
    int  defaultIndex() const;

    QComboBox* combo() const;

    void addItem(const QString& t, int index = -1);
    void insertItem(int index, const QString& t);

Q_SIGNALS:

    void reset();
    void activated(int);
    void currentIndexChanged(int);

public Q_SLOTS:

    void slotReset();

private Q_SLOTS:

    void slotItemActivated(int);
    void slotCurrentIndexChanged(int);

private:

    class Private;
    Private* const d;
};

}  // namespace KDcrawIface

#endif /* RCOMBOBOX_H */
