/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2007
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KIS_KRA_LOADER_H
#define KIS_KRA_LOADER_H

class QString;

#include "KoXmlReaderForward.h"
class KoStore;

class KisDoc2;
class KisNode;
class KoColorSpace;

#include <kis_types.h>

/**
 * Load old-style 1.x .kra files. Updated for 2.0, let's try to stay
 * compatible. But 2.0 won't be able to save 1.x .kra files unless we
 * implement an export filter.
 */
class KisKraLoader
{

public:

    KisKraLoader(KisDoc2 * document, int syntaxVersion);

    ~KisKraLoader();

    /**
     * Loading is done in two steps: first all xml is loaded, then, in finishLoading,
     * the actual layer data is loaded.
     */
    KisImageWSP loadXML(const KoXmlElement& elem);

    void loadBinaryData(KoStore* store, KisImageWSP image, const QString & uri, bool external);

private:

    KisNode* loadNodes(const KoXmlElement& element, KisImageWSP image, KisNode* parent);

    KisNode* loadNode(const KoXmlElement& elem, KisImageWSP image);

    KisNode* loadPaintLayer(const KoXmlElement& elem, KisImageWSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNode* loadGroupLayer(const KoXmlElement& elem, KisImageWSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNode* loadAdjustmentLayer(const KoXmlElement& elem, KisImageWSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNode* loadShapeLayer(const KoXmlElement& elem, KisImageWSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNode* loadGeneratorLayer(const KoXmlElement& elem, KisImageWSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNode* loadCloneLayer(const KoXmlElement& elem, KisImageWSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNode* loadFilterMask(const KoXmlElement& elem);

    KisNode* loadTransparencyMask(const KoXmlElement& elem);

    KisNode* loadTransformationMask(const KoXmlElement& elem);

    KisNode* loadSelectionMask(KisImageWSP image, const KoXmlElement& elem);


private:

    class Private;
    Private * const m_d;

};

#endif
