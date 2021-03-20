/*
 *  SPDX-FileCopyrightText: 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_LAYERCOMPOSITION_H
#define _KIS_LAYERCOMPOSITION_H

#include "kritaimage_export.h"

#include <QMap>
#include <QUuid>
#include <QDomDocument>
#include <QDomElement>

#include "kis_image.h"

/**
 * Storage class for layer compositions. Layer compositions allow to have several states for visible layers
 * e.g. used in storyboarding with one background and different foregrounds
 */
class KRITAIMAGE_EXPORT KisLayerComposition
{
public:
    KisLayerComposition(KisImageWSP image, const QString& name);
    ~KisLayerComposition();

    KisLayerComposition(const KisLayerComposition &rhs, KisImageWSP otherImage = 0);

   /**
    * Sets name of the composition
    */
    void setName(const QString& name);

   /**
    * Name of the composition as show in the docker
    * \return name of the composition
    */
    QString name();

   /**
    * Stores the current visibility of all layers in the composition
    */
    void store();
    
   /**
    * Applies the stored visibility to all the nodes
    */
    void apply();

   /**
    * Set the export enabled flag, if false the compositions will not be exported
    */
    void setExportEnabled(bool enabled);

   /**
    * Export enabled flag, if false the compositions will not be exported
    * \return name of the composition
    */
    bool isExportEnabled();

    void setVisible(QUuid id, bool visible);

    void setCollapsed(QUuid id, bool collapsed);

    void save(QDomDocument& doc, QDomElement& element);

private:
    KisImageWSP m_image;
    QString m_name;
    QMap<QUuid, bool> m_visibilityMap;
    QMap<QUuid, bool> m_collapsedMap;
    bool m_exportEnabled;
    
    friend class KisCompositionVisitor;
};

#endif
