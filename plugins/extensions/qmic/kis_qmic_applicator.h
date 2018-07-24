/*
 * Copyright (c) 2013-2014 Lukáš Tvrdý <lukast.dev@gmail.com
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

#ifndef _KIS_GMIC_APPLICATOR
#define _KIS_GMIC_APPLICATOR

#include <kundo2magicstring.h>

#include <kis_types.h>

#include <QThread>

#include "gmic.h"
#include "kis_qmic_data.h"

class KisProcessingApplicator;

class KisQmicApplicator : public QObject
{
    Q_OBJECT

public:
    KisQmicApplicator();
    ~KisQmicApplicator();
    void setProperties(KisImageWSP image, KisNodeSP node, QVector<gmic_image<float> *> images, const KUndo2MagicString &actionName, KisNodeListSP kritaNodes);

    void apply();
    void cancel();
    void finish();

    float getProgress() const;

Q_SIGNALS:
    void gmicFinished(bool successfully, int milliseconds = -1, const QString &msg = QString());

private:
    KisProcessingApplicator *m_applicator;
    KisImageWSP m_image;
    KisNodeSP m_node;
    KUndo2MagicString m_actionName;
    KisNodeListSP m_kritaNodes;
    bool m_applicatorStrokeEnded;
    QVector<gmic_image<float> *> m_images;
    KisQmicDataSP m_gmicData;
};

#endif


