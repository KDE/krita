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

#ifndef KO_PAGE_LAYOUT_DIALOG
#define KO_PAGE_LAYOUT_DIALOG

#include "kritawidgets_export.h"

#include <kpagedialog.h>
#include <KoUnit.h>

struct KoPageLayout;

/// A dialog to show the settings for one page and apply them afterwards.
class KRITAWIDGETS_EXPORT KoPageLayoutDialog : public KPageDialog
{
    Q_OBJECT
public:
    explicit KoPageLayoutDialog(QWidget *parent, const KoPageLayout &layout);
    ~KoPageLayoutDialog() override;

    void showTextDirection(bool on);
    void showPageSpread(bool on);
    void setPageSpread(bool pageSpread);
    KoPageLayout pageLayout() const;
    bool applyToDocument() const;
    void showApplyToDocument(bool on);

    void showUnitchooser(bool on);
    void setUnit(const KoUnit &unit);

Q_SIGNALS:
    void unitChanged(const KoUnit &unit);

public Q_SLOTS:
    void setPageLayout(const KoPageLayout &layout);

protected Q_SLOTS:
    void accept() override;
    void reject() override;

private:
    class Private;
    Private * const d;
};

#endif
