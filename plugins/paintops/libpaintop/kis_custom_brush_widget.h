/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#ifndef KIS_CUSTOM_BRUSH_H_
#define KIS_CUSTOM_BRUSH_H_

#include <QObject>
#include <QShowEvent>

#include <KoResourceServerAdapter.h>

#include "ui_wdgcustombrush.h"
#include <kis_types.h>
#include <kis_brush.h>

const QString TEMPORARY_FILENAME = "/tmp/temporaryKritaBrush.gbr";
const QString TEMPORARY_BRUSH_NAME = "Temporary custom brush";
const double DEFAULT_SPACING = 0.25;


class KoResource;

class KisWdgCustomBrush : public QDialog, public Ui::KisWdgCustomBrush
{
    Q_OBJECT

public:
    KisWdgCustomBrush(QWidget *parent) : QDialog(parent) {
        setupUi(this);
    }
};

class KisCustomBrushWidget : public KisWdgCustomBrush
{
    Q_OBJECT
public:
    KisCustomBrushWidget(QWidget *parent, const QString& caption, KisImageWSP image);
    virtual ~KisCustomBrushWidget();
    KisBrushSP brush();

    void setImage(KisImageWSP image);

protected:
    virtual void showEvent(QShowEvent *);

private Q_SLOTS:
    void slotAddPredefined();
    void slotUpdateCurrentBrush(int i = 0); // To connect with activated(int)
    void slotSpacingChanged();
    void slotUpdateUseColorAsMask(bool useColorAsMask);

Q_SIGNALS:
    void sigNewPredefinedBrush(KoResource *);

private:
    void createBrush();
    void updatePreviewImage();

    KisImageWSP m_image;
    KisBrushSP m_brush;

    QSharedPointer<KoAbstractResourceServerAdapter> m_rServerAdapter;
};


#endif // KIS_CUSTOM_BRUSH_H_
