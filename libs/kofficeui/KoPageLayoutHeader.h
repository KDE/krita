/* This file is part of the KDE project
 * Copyright (C) 2005 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef kopagelayoutheader_h
#define kopagelayoutheader_h

#include <KoUnit.h>
#include <KoPageLayout.h>
#include "ui_KoPageLayoutHeaderBase.h"

#include <QWidget>

class KoUnitDoubleSpinBox;
class KoPagePreview;

/**
 * This class is a widget that shows the KoKWHeaderFooter data structure and allows the user to change it.
 */
class KoPageLayoutHeader : public QWidget, public Ui::KoPageLayoutHeaderBase
{
    Q_OBJECT

public:
    /**
     * Contructor
     * @param parent the parent widget
     * @param unit the unit-type (mm/cm/inch) that the dialog should show
     * @param kwhf the data that this widget will be filled with initially
     */
    KoPageLayoutHeader(QWidget *parent, KoUnit::Unit unit, const KoKWHeaderFooter &kwhf);
    /**
     * @return the altered data as it is currently set by the user.
     */
    const KoKWHeaderFooter& headerFooter();

private:
    KoUnitDoubleSpinBox *m_headerSpacing, *m_footerSpacing, *m_footnoteSpacing;

    KoKWHeaderFooter m_headerFooters;
};

#endif

