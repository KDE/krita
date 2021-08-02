/*
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_PATTERN_CHOOSER_H_
#define KIS_PATTERN_CHOOSER_H_

#include <QFrame>

#include <KoResource.h>

#include <kritaui_export.h>

class KSqueezedTextLabel;
class KisResourceItemChooser;


class KRITAUI_EXPORT KisPatternChooser : public QFrame
{

    Q_OBJECT

public:
    KisPatternChooser(QWidget *parent = 0);
    ~KisPatternChooser() override;

    /// Gets the currently selected resource
    /// @returns the selected resource, 0 is no resource is selected
    KoResourceSP currentResource();
    void setGrayscalePreview(bool grayscale);
    /// determines whether the preview right or below the splitter
    void setPreviewOrientation(Qt::Orientation orientation);

Q_SIGNALS:

    /// Emitted when a resource was selected
    void resourceSelected(KoResourceSP resource);
    void updateItemSize();

public Q_SLOTS:

    void setCurrentPattern(KoResourceSP resource);
    void setCurrentItem(int row);

private Q_SLOTS:

    void update(KoResourceSP resource);

private:

    KSqueezedTextLabel *m_lblName;
    KisResourceItemChooser *m_itemChooser;
};

#endif // KIS_PATTERN_CHOOSER_H_

