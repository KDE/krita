/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
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
#ifndef KIS_CLIPBOARD_BRUSH_WIDGET_H
#define KIS_CLIPBOARD_BRUSH_WIDGET_H

//Qt includes
#include <QObject>
#include <QShowEvent>

//Calligra includes
#include <KoResourceServerAdapter.h>

//Krita includes
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

protected:
    void showEvent(QShowEvent *);

Q_SIGNALS:
    void sigNewPredefinedBrush(KoResource *);

private:
    KisClipboard* m_clipboard;
    KisPaintDeviceSP pd;
    KisBrushSP m_brush;
    QSharedPointer<KoAbstractResourceServerAdapter> m_rServerAdapter;
};

#endif // KIS_CLIPBOARD_BRUSH_WIDGET_H
