/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

// Description: Page Layout Dialog (header)

#ifndef __KOPGLAYOUTDIA_H__
#define __KOPGLAYOUTDIA_H__

#include <QGroupBox>
#include <KoGlobal.h>
#include <KoUnit.h>
#include <KoPageLayout.h>
#include <kdialogbase.h>

class QComboBox;
class QLineEdit;
class QCheckBox;
class KoUnitDoubleSpinBox;
class KoPageLayoutColumns;
class KoPageLayoutSize;
class KoPageLayoutHeader;

enum { FORMAT_AND_BORDERS = 1, HEADER_AND_FOOTER = 2, COLUMNS = 4, DISABLE_BORDERS = 8,
       KW_HEADER_AND_FOOTER = 16, DISABLE_UNIT = 32 };

/**
 *  KoPagePreview.
 *  Internal to KoPageLayoutDia.
 */
class KoPagePreview : public QGroupBox
{
    Q_OBJECT

public:

    /**
     *  The constructor
     */
    KoPagePreview( QWidget*, const char*, const KoPageLayout & );
    /**
     *  destructor
     */
    ~KoPagePreview();

    /**
     *  set page layout
     */
    void setPageLayout( const KoPageLayout& );
    /**
     * set page columns
     */
    void setPageColumns( const KoColumns& );

protected:

    // paint page
    void drawContents( QPainter* );

    double m_pageHeight, m_pageWidth, m_textFrameX, m_textFrameY, m_textFrameWidth, m_textFrameHeight;
    int columns;
};

class KoPageLayoutDiaPrivate;

/**
 *  With this dialog the user can specify the layout of the paper during printing.
 */
class KOFFICEUI_EXPORT KoPageLayoutDia : public KDialogBase
{
    Q_OBJECT

public:

    /**
     *  Constructor.
     *
     *  @param parent   The parent of the dialog.
     *  @param name     The name of the dialog.
     *  @param layout   The layout.
     *  @param headfoot The header and the footer.
     *  @param flags     a variable with all features this dialog should show.
     *  @param unit     The unit to use for displaying the values to the user.
     *  @param modal    Whether the dialog is modal or not.
     */
    KoPageLayoutDia( QWidget* parent, const char* name,
             const KoPageLayout& layout,
             const KoHeadFoot& headfoot,
             int flags, KoUnit::Unit unit, bool modal=true );

    /**
     *  Constructor.
     *
     *  @param parent     The parent of the dialog.
     *  @param name       The name of the dialog.
     *  @param layout     The layout.
     *  @param headfoot   The header and the footer.
     *  @param columns    The number of columns on the page.
     *  @param kwheadfoot The KWord header and footer.
     *  @param tabs       The number of tabs.
     *  @param unit       The unit to use for displaying the values to the user
     */
    KoPageLayoutDia( QWidget* parent, const char* name,
             const KoPageLayout& layout,
             const KoHeadFoot& headfoot,
             const KoColumns& columns,
             const KoKWHeaderFooter& kwheadfoot,
             int tabs, KoUnit::Unit unit );

    /**
     *  Destructor.
     */
    ~KoPageLayoutDia();

    /**
     *  Show page layout dialog.
     *  See constructor for documentation on the parameters
     */
    static bool pageLayout( KoPageLayout&, KoHeadFoot&, int tabs, KoUnit::Unit& unit, QWidget* parent = 0 );

    /**
     *  Show page layout dialog.
     *  See constructor for documentation on the parameters
     */
    static bool pageLayout( KoPageLayout&, KoHeadFoot&, KoColumns&, KoKWHeaderFooter&, int tabs, KoUnit::Unit& unit, QWidget* parent = 0 );
    /**
     *  Retrieves a standard page layout.
     *  Deprecated: better use KoPageLayout::standardLayout()
     */
    static KDE_DEPRECATED KoPageLayout standardLayout();

    /**
     *  Returns the layout
     */
    const KoPageLayout& layout() const { return m_layout; }

    /**
     *  Returns the header and footer information
     */
    KoHeadFoot headFoot() const;

    /**
     *  Returns the unit
     */
    KoUnit::Unit unit() const { return m_unit; }

private:
    const KoColumns& columns() { return m_column; }
    const KoKWHeaderFooter& headerFooter();

    // setup tabs
    void setupTab1( bool enableBorders );
    void setupTab2( const KoHeadFoot& hf );
    void setupTab3();
    void setupTab4( const KoKWHeaderFooter kwhf );

    // dialog objects
    QLineEdit *eHeadLeft;
    QLineEdit *eHeadMid;
    QLineEdit *eHeadRight;
    QLineEdit *eFootLeft;
    QLineEdit *eFootMid;
    QLineEdit *eFootRight;

    // layout
    KoPageLayout m_layout;
    KoColumns m_column;

    KoUnit::Unit m_unit;

    int flags;

protected slots:
    virtual void slotOk();

private slots:
    void sizeUpdated(KoPageLayout &layout);
    void columnsUpdated(KoColumns &columns);

private:
    KoPageLayoutSize *m_pageSizeTab;
    KoPageLayoutColumns *m_columnsTab;
    KoPageLayoutHeader *m_headerTab;
    KoPageLayoutDiaPrivate *d;
};

#endif
