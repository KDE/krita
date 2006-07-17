/* This file is part of the KDE project
 * Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
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

// Description: Page Layout Dialog (sources)

#ifndef kopagelayoutcolumns_h
#define kopagelayoutcolumns_h

#include <KoUnit.h>
#include <KoPageLayout.h>
#include "ui_KoPageLayoutColumns.h"

class QWidget;
class KoUnitDoubleSpinBox;
class KoPagePreview;

/**
 * This class is a widget that shows the KoColumns data structure and allows the user to change it.
 */
class KOFFICEUI_EXPORT KoPageLayoutColumns : public QWidget, public Ui::KoPageLayoutColumns {
    Q_OBJECT

public:
    /**
     * Contructor
     * @param parent the parent widget
     * @param columns the KoColumns data structure that this dialog should be initialzed with
     * @param unit the unit-type (mm/cm/inch) that the dialog should show
     * @param layout the page layout that the preview should be initialzed with.
     */
    KoPageLayoutColumns(QWidget *parent, const KoColumns& columns, KoUnit::Unit unit, const KoPageLayout& layout);

    /**
     * Update the page preview widget with the param layout.
     * @param layout the new layout
     */
    void setLayout(KoPageLayout &layout);
public slots:

    /**
     * Enable the user to edit the columns
     * @param on if true enable the user to change the columns count
     */
    void setEnableColumns(bool on);

signals:
    void propertyChange(KoColumns &columns);

protected:
    KoColumns m_columns;
    KoPagePreview *m_preview;
    KoUnitDoubleSpinBox *m_spacing;

private slots:
    void nColChanged( int );
    void nSpaceChanged( double );
};

#endif
