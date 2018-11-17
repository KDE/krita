/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
 *  Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
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
#ifndef KIS_ITEM_CHOOSER_H_
#define KIS_ITEM_CHOOSER_H_

#include <QWidget>
#include <QScroller>

#include <KoResource.h>
#include <KoID.h>

class KoAbstractResourceServerAdapter;
class KisPresetDelegate;
class KoResourceItemChooser;

#include <kritaui_export.h>

/**
 * A special type of item chooser that can contain extra widgets that show
 * more information about the currently selected item. Reimplement update()
 * to extract that information and fill the appropriate widgets.
 */
class KRITAUI_EXPORT KisPresetChooser : public QWidget
{
    Q_OBJECT

public:

    KisPresetChooser(QWidget *parent = 0, const char *name = 0);
    ~KisPresetChooser() override;

    enum ViewMode{
        THUMBNAIL, /// Shows thumbnails
        DETAIL,  /// Shows thumbsnails with text next to it
        STRIP  /// Shows thumbnails arranged in a single row
    };

    /// Sets a list of resources in the paintop list, when ever user press enter in the linedit of paintop_presets_popup Class
    void setViewMode(ViewMode mode);
    void showButtons(bool show);

    void setCurrentResource(KoResourceSP resource);
    KoResourceSP currentResource() const;
    /// Sets the visibility of tagging klineEdits
    void showTaggingBar(bool show);
    KoResourceItemChooser *itemChooser();
    void setPresetFilter(const QString& paintOpId);

    /// get the base size for the icons. Used by the slider in the view options
    int iconSize();

Q_SIGNALS:
    void resourceSelected(KoResourceSP resource);
    void resourceClicked(KoResourceSP resource);

public Q_SLOTS:
    void updateViewSettings();

    /// sets the icon size. Used by slider in view menu
    void setIconSize(int newSize);

    /// saves the icon size for the presets. called by the horizontal slider release event.
    void saveIconSize();

    void slotScrollerStateChanged(QScroller::State state);

private Q_SLOTS:
    void notifyConfigChanged();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    KoResourceItemChooser *m_chooser;
    KisPresetDelegate* m_delegate;
    ViewMode m_mode;
    QSharedPointer<KoAbstractResourceServerAdapter> m_adapter;
};

#endif // KIS_ITEM_CHOOSER_H_

