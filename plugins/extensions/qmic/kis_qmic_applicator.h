/*
 * SPDX-FileCopyrightText: 2013-2014 Lukáš Tvrdý <lukast.dev@gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_GMIC_APPLICATOR
#define _KIS_GMIC_APPLICATOR

#include <kundo2magicstring.h>

#include <kis_types.h>

#include <QThread>

#include "gmic.h"
#include "kis_qmic_data.h"
#include <QScopedPointer>

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
    QScopedPointer<KisProcessingApplicator> m_applicator;
    KisImageWSP m_image;
    KisNodeSP m_node;
    KUndo2MagicString m_actionName;
    KisNodeListSP m_kritaNodes;
    QVector<gmic_image<float> *> m_images;
    KisQmicDataSP m_gmicData;
};

#endif


