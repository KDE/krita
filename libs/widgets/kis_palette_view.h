/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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

#ifndef __KIS_PALETTE_VIEW_H
#define __KIS_PALETTE_VIEW_H

#include <QScopedPointer>

#include <QColorDialog>
#include <QPushButton>
#include <QPixmap>
#include <QIcon>

#include <KoTableView.h>
#include <KoColorSet.h>
#include "kritawidgets_export.h"

class KisPaletteModel;
class QWheelEvent;


class KRITAWIDGETS_EXPORT KisPaletteView : public KoTableView
{
    Q_OBJECT
public:
    KisPaletteView(QWidget *parent = 0);
    ~KisPaletteView() override;

    void setPaletteModel(KisPaletteModel *model);
    KisPaletteModel* paletteModel() const;

    /**
     * @brief updateRows
     * update the rows so they have the proper columnspanning.
     */
    void updateRows();

    /**
     * @brief setAllowModification
     * Set whether doubleclick calls up a modification window. This is to prevent users from editing
     * the palette when the palette is intended to be a list of items.
     */
    void setAllowModification(bool allow);

    /**
     * @brief setCrossedKeyword
     * this apparently allows you to set keywords that can cross out colors.
     * This is implemented to mark the lazybrush "transparent" color.
     * @param value
     */
    void setCrossedKeyword(const QString &value);

public Q_SLOTS:
    void paletteModelChanged();
    /**
     * add an entry with a dialog window.
     */
    bool addEntryWithDialog(KoColor color);
    /**
     * @brief addGroupWithDialog
     * summons a little dialog to name the new group.
     */
    bool addGroupWithDialog();
    /**
     * remove entry with a dialog window.(Necessary for groups.
     */
    bool removeEntryWithDialog(QModelIndex index);
    /**
     *  This tries to select the closest color in the palette.
     *  This doesn't update the foreground color, just the visual selection.
     */
    void trySelectClosestColor(KoColor color);
Q_SIGNALS:
    /**
     * @brief entrySelected
     * signals when an entry is selected.
     * @param entry the selected entry.
     */
    void entrySelected(KoColorSetEntry entry);
    void entrySelectedBackGround(KoColorSetEntry entry);
    void indexEntrySelected(QModelIndex index);
protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
private Q_SLOTS:
    /**
     * @brief entrySelection
     * the function that will emit entrySelected when the entry changes.
     */
    void entrySelection(bool foreground = true);
    /**
     * @brief modifyEntry
     * function for changing the entry at the given index.
     * if modification isn't allow(@see setAllowModification), this does nothing.
     */
    void modifyEntry(QModelIndex index);
};

#endif /* __KIS_PALETTE_VIEW_H */
