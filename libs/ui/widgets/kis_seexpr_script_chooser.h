/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_SEEXPR_SCRIPT_CHOOSER_H_
#define KIS_SEEXPR_SCRIPT_CHOOSER_H_

#include <QFrame>
#include <kritaui_export.h>
#include <KSqueezedTextLabel>
#include <KisResourceItemChooser.h>

class KRITAUI_EXPORT KisSeExprScriptChooser : public QFrame
{

    Q_OBJECT

public:
    KisSeExprScriptChooser(QWidget *parent = 0);
    ~KisSeExprScriptChooser() override;

    /// Gets the currently selected resource
    /// @returns the selected resource, 0 is no resource is selected
    KoResourceSP currentResource();
    void setCurrentScript(KoResourceSP resource);
    void setCurrentItem(int row);
    /// determines whether the preview right or below the splitter
    void setPreviewOrientation(Qt::Orientation orientation);

Q_SIGNALS:

    /// Emitted when a resource was selected
    void resourceSelected(KoResourceSP resource);
    void updateItemSize();

private Q_SLOTS:

    void update(KoResourceSP resource);

private:

    KSqueezedTextLabel *m_lblName;
    KisResourceItemChooser *m_itemChooser;
};

#endif // KIS_SEEXPR_SCRIPT_CHOOSER_H_

