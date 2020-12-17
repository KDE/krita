/*
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SHAPE_SELECTION_MODEL_H
#define KIS_SHAPE_SELECTION_MODEL_H

#include <QObject>
#include <QRect>
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
    ~KisShapeSelectionModel() override;

    void add(KoShape *child) override;
    void remove(KoShape *child) override;

    void setUpdatesEnabled(bool enabled);
    bool updatesEnabled() const;

    void setClipped(const KoShape *child, bool clipping) override;
    bool isClipped(const KoShape *child) const override;
    void setInheritsTransform(const KoShape *shape, bool inherit) override;
    bool inheritsTransform(const KoShape *shape) const override;

    int count() const override;
    QList<KoShape*> shapes() const override;

    void containerChanged(KoShapeContainer *, KoShape::ChangeType) override;
    void childChanged(KoShape * child, KoShape::ChangeType type) override;
    void setShapeSelection(KisShapeSelection* selection);

private Q_SLOTS:
    void requestUpdate(const QRect &updateRect);

private:
    QMap<KoShape*, QRectF> m_shapeMap;
    KisImageWSP m_image;
    KisSelectionWSP m_parentSelection;
    KisShapeSelection* m_shapeSelection;

    bool m_updatesEnabled;
};

#endif
