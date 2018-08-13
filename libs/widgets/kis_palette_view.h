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
#include <QTableView>
#include <QColorDialog>
#include <QPushButton>
#include <QPixmap>
#include <QIcon>

#include <KoColorSet.h>
#include "kritawidgets_export.h"

class KisPaletteModel;
class QWheelEvent;
class KoColorDisplayRendererInterface;

class KRITAWIDGETS_EXPORT KisPaletteView : public QTableView
{
    Q_OBJECT
private:
    static int MININUM_ROW_HEIGHT;
public:
    explicit KisPaletteView(QWidget *parent = Q_NULLPTR);
    ~KisPaletteView() override;

    void setPaletteModel(KisPaletteModel *model);
    KisPaletteModel* paletteModel() const;

public:

    /**
     * @brief setAllowModification
     * Set whether doubleclick calls up a modification window. This is to prevent users from editing
     * the palette when the palette is intended to be a list of items.
     */
    void setAllowModification(bool allow);

    void setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer);

    /**
     * @brief setCrossedKeyword
     * this apparently allows you to set keywords that can cross out colors.
     * This is implemented to mark the lazybrush "transparent" color.
     * @param value
     */
    void setCrossedKeyword(const QString &value);
    void removeSelectedEntry();
    /**
     * @brief selectClosestColor
     * select a color that's closest to parameter color
     * @param color
     */
    void selectClosestColor(const KoColor &color);

    /**
     * add an entry with a dialog window.
     * @warning deprecated.
     * kept for compatibility with @ref PaletteView in @ref libkis
     */
    bool addEntryWithDialog(KoColor color);
    /**
     * remove entry with a dialog window.(Necessary for groups.
     * @warning deprecated.
     * kept for compatibility with @ref PaletteView in @ref libkis
     */
    bool removeEntryWithDialog(QModelIndex index);
    /**
     * add entry with a dialog window.
     * @warning deprecated.
     * kept for compatibility with @ref PaletteView in @ref libkis
     */
    bool addGroupWithDialog();

Q_SIGNALS:
    void sigIndexSelected(const QModelIndex &index);
    void sigColorSelected(const KoColor &);

public Q_SLOTS:
    /**
     *  This tries to select the closest color in the palette.
     *  This doesn't update the foreground color, just the visual selection.
     */
    void slotFGColorChanged(const KoColor &);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private Q_SLOTS:
    void slotHorizontalHeaderResized(int, int, int newSize);
    void slotAdditionalGuiUpdate();

private:
    void resizeRows(int newSize);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_PALETTE_VIEW_H */
