/*
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef _KIS_LAYERCOMPOSITION_H
#define _KIS_LAYERCOMPOSITION_H

#include "kritaimage_export.h"

#include <QMap>
#include <QUuid>
#include <QDomDocument>
#include <QDomElement>

#include "kis_image.h"

/**
 * Storage class for layer compositions. Layer compositions allow to have several state for visible layers
 * e.g. used in storyboarding with one background and differnt foregrounds
 */
class KRITAIMAGE_EXPORT KisLayerComposition
{
public:
    KisLayerComposition(KisImageWSP image, const QString& name);
    ~KisLayerComposition();

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
