/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    bool eventFilter(QObject *watched, QEvent *event) override;

private Q_SLOTS:
    void slotCanvasResourceChanged(int key, const QVariant &resource);
    void slotReloadProperties();
    void slotConfigBrushHud();
    void slotReloadPreset();

private:
    void clearProperties() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_BRUSH_HUD_H */
