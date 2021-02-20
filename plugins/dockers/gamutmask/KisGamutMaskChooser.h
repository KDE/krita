/*
 *  SPDX-FileCopyrightText: 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KISGAMUTMASKCHOOSER_H
#define KISGAMUTMASKCHOOSER_H

#include <QWidget>

class KisResourceItemChooser;
#include <KoResource.h>
#include <resources/KoGamutMask.h>

class KisGamutMaskDelegate;

class KisGamutMaskChooser : public QWidget
{
    Q_OBJECT
public:
    explicit KisGamutMaskChooser(QWidget *parent = nullptr);
    ~KisGamutMaskChooser() override;

    enum ViewMode {
        THUMBNAIL, // Shows thumbnails
        DETAIL  // Shows thumbsnails with text next to it
    };

    void setCurrentResource(KoResourceSP resource);

protected:
    void resizeEvent(QResizeEvent* event) override;

Q_SIGNALS:
    void sigGamutMaskSelected(KoGamutMaskSP mask);

private Q_SLOTS:
    void resourceSelected(KoResourceSP resource);
    void slotSetModeThumbnail();
    void slotSetModeDetail();

private:
    void setViewMode(ViewMode mode);
    void updateViewSettings();
    KisResourceItemChooser* m_itemChooser;
    KisGamutMaskDelegate* m_delegate;
    ViewMode m_mode;
};

#endif // KISGAMUTMASKCHOOSER_H
