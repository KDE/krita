/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

