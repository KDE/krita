/*
 * SPDX-FileCopyrightText: 2023 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __KISCOLORPATCHESTABLEVIEW_H_
#define __KISCOLORPATCHESTABLEVIEW_H_

#include <QAbstractItemModel>
#include <QScroller>
#include <QTableView>

#include <KisKineticScroller.h>
#include <KoColor.h>

class KoColor;
class KisCanvas2;

class KisColorPatchesTableView : public QTableView
{
    Q_OBJECT
public:
    KisColorPatchesTableView(const QString &configPrefix, QWidget *parent = nullptr);
    ~KisColorPatchesTableView() override;

    void setColors(const QList<KoColor> &colors);
    void addColorPatch(const KoColor &color);
    QList<KoColor> colors() const;
    boost::optional<KoColor> colorPatchAt(const QPoint &pos) const;

    QSize cellSize() const;
    int patchCount() const;
    void reloadWidgetConfig();

    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

public Q_SLOTS:
    void slotScrollerStateChanged(QScroller::State state)
    {
        KisKineticScroller::updateCursor(this, state);
    }

private:
    void redraw();

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // __KISCOLORPATCHESTABLEVIEW_H_
