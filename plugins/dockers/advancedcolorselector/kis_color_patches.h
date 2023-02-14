/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_COLOR_PATCHES_H
#define KIS_COLOR_PATCHES_H

#include "kis_color_selector_base.h"

#include "KoColor.h"

class KoColor;
class KisColorPatchesTableView;


class KisColorPatches : public KisColorSelectorBase
{
Q_OBJECT
public:
    explicit KisColorPatches(QString configPrefix, QWidget *parent = 0);
    enum Direction { Horizontal, Vertical };

public Q_SLOTS:
    void updateSettings() override;

public:
    void setColors(const QList<KoColor> &colors);
    QList<KoColor> colors() const;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

    int patchCount() const;

    void setCanvas(KisCanvas2 *canvas) override;
    void unsetCanvas() override;

    void addColorPatch(const KoColor &color);

public:
    /// set buttons, that should be drawn additionally to the patches
    /// this class takes ownership of them and will delete them
    /// they will be resized to the patchsize
    void setAdditionalButtons(QList<QWidget*> buttonList);

private:

    Direction m_direction;
    QList<QWidget*> m_buttonList;
    QString m_configPrefix;
    KisColorPatchesTableView *m_colorPatchesView;
};

#endif
