/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef KOFFICE_PAGE_LAYOUT_WIDGET
#define KOFFICE_PAGE_LAYOUT_WIDGET

#include "kowidgets_export.h"

#include <KoText.h>
#include <KoUnit.h>
#include <KoPageLayout.h>
#include <QWidget>

/// the widget that shows the size/margins and other page settings.
class KOWIDGETS_EXPORT KoPageLayoutWidget : public QWidget
{
    Q_OBJECT

public:

    KoPageLayoutWidget(QWidget *parent, const KoPageLayout &layout);
    ~KoPageLayoutWidget();

    void setUnit(const KoUnit &unit);
    void showUnitchooser(bool on);
    void showPageSpread(bool on);
    void setPageSpread(bool pageSpread);
    void showTextDirection(bool on);
    void setTextDirection(KoText::Direction direction);
    KoText::Direction textDirection() const;

signals:
    void layoutChanged(const KoPageLayout &layout);
    void unitChanged(const KoUnit &unit);

public slots:
    void setPageLayout(const KoPageLayout &layout);
    void setTextAreaAvailable(bool available);

private slots:
    void sizeChanged(int row);
    void unitChanged(int row);
    void facingPagesChanged();
    void optionsChanged();
    void marginsChanged();
    void orientationChanged();
    void setApplyToDocument(bool apply);

private:
    class Private;
    Private * const d;
};

#endif
