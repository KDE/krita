/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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
#ifndef KIS_PATTERN_CHOOSER_H_
#define KIS_PATTERN_CHOOSER_H_

#include <QFrame>

#include <KoResource.h>

#include <kritaui_export.h>

class KSqueezedTextLabel;
class KoResourceItemChooser;


class KRITAUI_EXPORT KisPatternChooser : public QFrame
{

    Q_OBJECT

public:
    KisPatternChooser(QWidget *parent = 0);
    ~KisPatternChooser() override;

    /// Gets the currently selected resource
    /// @returns the selected resource, 0 is no resource is selected
    KoResourceSP currentResource();
    void setCurrentPattern(KoResourceSP resource);
    void setCurrentItem(int row, int column);
    void setGrayscalePreview(bool grayscale);
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
    KoResourceItemChooser *m_itemChooser;
};

#endif // KIS_PATTERN_CHOOSER_H_

