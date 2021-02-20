/*
 *  SPDX-FileCopyrightText: 2005 Bart Coppens <kde@bartcoppens.be>
 *  SPDX-FileCopyrightText: 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_CLIPBOARD_BRUSH_WIDGET_H
#define KIS_CLIPBOARD_BRUSH_WIDGET_H

#include <QObject>
#include <QShowEvent>

#include <KoResourceServer.h>

#include <kis_types.h>
#include <kis_brush.h>
#include "ui_wdgclipboardbrush.h"

const QString TEMPORARY_CLIPBOARD_BRUSH_FILENAME = "/tmp/temporaryClipboardBrush.gbr";
const QString TEMPORARY_CLIPBOARD_BRUSH_NAME = "Temporary clipboard brush";
const double DEFAULT_CLIPBOARD_BRUSH_SPACING = 0.25;

class KisClipboard;
class KoResource;

class KisWdgClipboardBrush : public QDialog, public Ui::KisWdgClipboardBrush
{
    Q_OBJECT

public:
    KisWdgClipboardBrush(QWidget* parent) : QDialog(parent) {
        setupUi(this);
    }
};

class KisClipboardBrushWidget : public KisWdgClipboardBrush
{
    Q_OBJECT
public:
    KisClipboardBrushWidget(QWidget* parent, const QString& caption, KisImageWSP image);
    virtual ~KisClipboardBrushWidget();

private Q_SLOTS:
    void slotCreateBrush();
    void slotSpacingChanged();
    void slotUpdateUseColorAsMask(bool useColorAsMask);
    void slotAddPredefined();
    void slotUpdateSaveButton();
    void slotClipboardContentChanged();

protected:
    void showEvent(QShowEvent *);

Q_SIGNALS:
    void sigNewPredefinedBrush(KoResourceSP );

private:
    KisClipboard* m_clipboard;
    KisPaintDeviceSP pd;
    KisBrushSP m_brush;
    KoResourceServer<KisBrush> *m_rServer;
};

#endif // KIS_CLIPBOARD_BRUSH_WIDGET_H
