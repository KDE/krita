/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_PROCESSING_APPLICATOR_H
#define __KIS_PROCESSING_APPLICATOR_H

#include "krita_export.h"
#include "kis_types.h"

#include "kis_stroke_job_strategy.h"
#include "kis_image_signal_router.h"


class KRITAIMAGE_EXPORT KisProcessingApplicator
{
public:
    KisProcessingApplicator(KisImageWSP image,
                            KisNodeSP node,
                            bool recursive = false,
                            KisImageSignalVector emitSignals = KisImageSignalVector(),
                            const QString &name = QString());

    ~KisProcessingApplicator();

    void applyVisitor(KisProcessingVisitorSP visitor,
                      KisStrokeJobData::Sequentiality sequentiality = KisStrokeJobData::SEQUENTIAL,
                      KisStrokeJobData::Exclusivity exclusivity = KisStrokeJobData::NORMAL);

    void applyCommand(KUndo2Command *command,
                      KisStrokeJobData::Sequentiality sequentiality = KisStrokeJobData::SEQUENTIAL,
                      KisStrokeJobData::Exclusivity exclusivity = KisStrokeJobData::NORMAL);

    void end();

private:
    void visitRecursively(KisNodeSP node,
                          KisProcessingVisitorSP visitor,
                          KisStrokeJobData::Sequentiality sequentiality,
                          KisStrokeJobData::Exclusivity exclusivity);

private:
    KisImageWSP m_image;
    KisNodeSP m_node;
    bool m_recursive;
    KisImageSignalVector m_emitSignals;
    KisStrokeId m_strokeId;
};

#endif /* __KIS_PROCESSING_APPLICATOR_H */
