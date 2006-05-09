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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef kopagelayoutsize_h
#define kopagelayoutsize_h

#include <KoGlobal.h>
#include <KoUnit.h>
#include <kdialogbase.h>
#include "KoPageLayout.h"
#include "KoPageLayoutDia.h"

class QGroupBox;
class QButtonGroup;
class QComboBox;
class KoUnitDoubleSpinBox;
class KoPageLayoutColumns;

/**
 * This class is a widget that shows the KoPageLayout data structure and allows the user to change it.
 */
class KOFFICEUI_EXPORT KoPageLayoutSize : public QWidget
{
    Q_OBJECT

public:
    /**
     * Contructor
     * @param parent the parent widget
     * @param layout the page layout that this widget should be initialzed with.
     * @param unit the unit-type (mm/cm/inch) that the dialog should show
     * @param columns the KoColumns (amout of columns) that the preview should be initialized with
     * @param unitChooser if true a combobox with the unit-type is shown for the user to change
     * @param enableBorders if true enable the user to change the margins (aka borders) of the page
     */
    KoPageLayoutSize(QWidget *parent, const KoPageLayout& layout, KoUnit::Unit unit,
            const KoColumns& columns, bool unitChooser, bool enableBorders);

    /**
     * @return if the dialog is in a sane state and the values can be used.
     */
    bool queryClose();
    /**
     * Update the page preview widget with the param columns.
     * @param columns the new columns
     */
    void setColumns(KoColumns &columns);

public slots:
    /**
     * Set a new unit for the widget updating the widgets.
     * @param unit the new unit
     */
    void setUnit( KoUnit::Unit unit );
    /**
     * Enable the user to edit the page border size
     * @param on if true enable the user to change the margins (aka borders) of the page
     */
    void setEnableBorders(bool on);

signals:
    /**
     * Emitted whenever the user changed something in the dialog.
     * @param layout the update layout structure with currently displayed info.
     * Note that the info may not be fully correct and physically possible (in which
     * case queryClose will return false)
     */
    void propertyChange(KoPageLayout &layout);

protected:
    QComboBox *cpgFormat;
    KoUnitDoubleSpinBox *epgWidth;
    KoUnitDoubleSpinBox *epgHeight;
    KoUnitDoubleSpinBox *ebrLeft;
    KoUnitDoubleSpinBox *ebrRight;
    KoUnitDoubleSpinBox *ebrTop;
    KoUnitDoubleSpinBox *ebrBottom;
    KoPagePreview *pgPreview;
    QGroupBox *m_orientBox;
    QButtonGroup* m_orientGroup;

protected slots:
    void formatChanged( int );
    void widthChanged( double );
    void heightChanged( double );
    void leftChanged( double );
    void rightChanged( double );
    void topChanged( double );
    void bottomChanged( double );
    void orientationChanged( int );
    void setUnitInt( int unit );

private:
    void updatePreview();
    void setValues();

    KoUnit::Unit m_unit;
    KoPageLayout m_layout;

    bool m_blockSignals, m_haveBorders;
};

#endif
