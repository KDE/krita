/* This file is part of the KDE project
 * Copyright (C) 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#ifndef KIS_COLORSET_CHOOSER_H
#define KIS_COLORSET_CHOOSER_H

#include <QWidget>

class QSpinBox;
class KoColorSet;
class QLineEdit;
class KoResourceItemChooser;
class KoResource;

#include "kritawidgets_export.h"


class KRITAWIDGETS_EXPORT KisColorsetChooser : public QWidget
{
    Q_OBJECT
public:
    KisColorsetChooser(QWidget* parent = 0);
    ~KisColorsetChooser() override;

Q_SIGNALS:
    void paletteSelected(KoColorSetSP colorSet);

private Q_SLOTS:
    void resourceSelected(KoResourceSP resource);
    void slotSave();

private:
    KoResourceItemChooser * m_itemChooser;
    QLineEdit* m_nameEdit;
    QSpinBox* m_columnEdit;
};

#endif // COLORSET_CHOOSER_H
