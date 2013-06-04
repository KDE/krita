/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef KIS_SHAPE_SELECTION_MODEL_H
#define KIS_SHAPE_SELECTION_MODEL_H

#include <QObject>
#include "KoShapeContainerModel.h"
#include "kis_types.h"
#include "kis_signal_compressor.h"

class KisShapeSelection;

/**
 *
 */
class KisShapeSelectionModel: public QObject, public KoShapeContainerModel
{
    Q_OBJECT
public:
    KisShapeSelectionModel(KisImageWSP image, KisSelectionWSP selection, KisShapeSelection* shapeSelection);
    ~KisShapeSelectionModel();

    void add(KoShape *child);
    void remove(KoShape *child);

    void setUpdatesEnabled(bool enabled);
    bool updatesEnabled() const;

    void setClipped(const KoShape *child, bool clipping);
    bool isClipped(const KoShape *child) const;
    virtual void setInheritsTransform(const KoShape *shape, bool inherit);
    virtual bool inheritsTransform(const KoShape *shape) const;

    int count() const;
    QList<KoShape*> shapes() const;

    void containerChanged(KoShapeContainer *, KoShape::ChangeType);
    void childChanged(KoShape * child, KoShape::ChangeType type);
    bool isChildLocked(const KoShape *child) const;
    void setShapeSelection(KisShapeSelection* selection);

private slots:
    void requestUpdate(const QRect &updateRect);
    void startUpdateJob();

private:
    QMap<KoShape*, QRectF> m_shapeMap;
    KisImageWSP m_image;
    KisSelectionWSP m_parentSelection;
    KisShapeSelection* m_shapeSelection;

    KisSignalCompressor m_updateSignalCompressor;
    QRect m_updateRect;
    bool m_updatesEnabled;
};

#endif
