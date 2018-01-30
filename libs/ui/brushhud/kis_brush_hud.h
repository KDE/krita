/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_BRUSH_HUD_H
#define __KIS_BRUSH_HUD_H

#include <QScopedPointer>
#include <QWidget>

class KisCanvasResourceProvider;

class KisBrushHud : public QWidget
{
    Q_OBJECT
public:
    KisBrushHud(KisCanvasResourceProvider *provider, QWidget *parent);
    ~KisBrushHud() override;

    void updateProperties();
    QSize sizeHint() const override;

    void updateIcons();

protected:
    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private Q_SLOTS:
    void slotCanvasResourceChanged(int key, const QVariant &resource);
    void slotReloadProperties();
    void slotConfigBrushHud();

private:
    void clearProperties() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_BRUSH_HUD_H */
