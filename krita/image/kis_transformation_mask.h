/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef _KIS_TRANSFORMATION_MASK_
#define _KIS_TRANSFORMATION_MASK_


#include <QUndoCommand>

#include <klocale.h>

#include "kis_types.h"
#include "kis_effect_mask.h"
#include <kis_filter_strategy.h>
#include <widgets/kis_cmb_idlist.h>

class KisFilterStrategy;


/**
 * A transformation mask applies a particular transformation to the
 * pixels of a paint device that are selected by the mask paint
 * device.
 */
class KRITAIMAGE_EXPORT KisTransformationMask : public KisEffectMask
{
    Q_OBJECT

public:

    /**
     * Create an empty filter mask.
     */
    KisTransformationMask();

    virtual ~KisTransformationMask();

    KisNodeSP clone() const {
        return KisNodeSP(new KisTransformationMask(*this));
    }


    KisTransformationMask(const KisTransformationMask& rhs);

    bool allowAsChild(KisNodeSP) const;

    void setXScale(double xscale) {
        m_xscale = xscale;
    }

    double xScale() {
        return m_xscale;
    }

    void setYScale(double yscale) {
        m_yscale = yscale;
    }

    double yScale() {
        return m_yscale;
    }

    void setXShear(double xshear) {
        m_xshear = xshear;
    }

    double xShear() {
        return m_xshear;
    }

    void setYShear(double yshear) {
        m_yshear = yshear;
    }

    double yShear() {
        return m_yshear;
    }

    void setRotation(double rotation) {
        m_rotation = rotation;
    }

    double rotation() {
        return m_rotation;
    }

    void setXTranslation(qint32 xtranslate) {
        m_xtranslate = xtranslate;
    }

    qint32 xTranslate() {
        return m_xtranslate;
    }

    void setYTranslation(qint32 ytranslate) {
        m_ytranslate = ytranslate;
    }

    qint32 yTranslate() {
        return m_ytranslate;
    }


    void setFilterStrategy(KisFilterStrategy * filter) {
        m_filter = filter;
    }

    KisFilterStrategy * filterStrategy() {
        return m_filter;
    }

    /**
     * Apply the effect the projection using the mask as a selection.
     */
    virtual void apply(KisPaintDeviceSP projection, const QRect & rc) const;

    QRect changeRect(const QRect &rect) const;
    QRect needRect(const QRect &rect, PositionToFilthy pos = NORMAL) const;

    bool accept(KisNodeVisitor &v);
private:

    double m_xscale, m_yscale;
    double m_xshear, m_yshear;
    double m_rotation;
    qint32 m_xtranslate, m_ytranslate;
    KisFilterStrategy * m_filter;

};

class KRITAIMAGE_EXPORT KisTransformationSettingsCommand : public QUndoCommand
{

public:
    /**
     *
     */
    KisTransformationSettingsCommand(KisTransformationMaskSP mask,
                                     const QString & old_name,
                                     double old_xscale,
                                     double old_yscale,
                                     double old_xshear,
                                     double old_yshear,
                                     double old_rotation,
                                     qint32 old_xtranslate,
                                     qint32 old_ytranslate,
                                     KisFilterStrategy * old_filter,
                                     const QString & new_name,
                                     double new_xscale,
                                     double new_yscale,
                                     double new_xshear,
                                     double new_yshear,
                                     double new_rotation,
                                     qint32 new_xtranslate,
                                     qint32 new_ytranslate,
                                     KisFilterStrategy * new_filter);

    virtual void redo();

    virtual void undo();

private:

    KisTransformationMaskSP m_mask;

    QString m_old_name;
    double m_old_xscale, m_old_yscale;
    double m_old_xshear, m_old_yshear;
    double m_old_rotation;
    qint32 m_old_xtranslate, m_old_ytranslate;
    KisFilterStrategy * m_old_filter;

    QString m_new_name;
    double m_new_xscale, m_new_yscale;
    double m_new_xshear, m_new_yshear;
    double m_new_rotation;
    qint32 m_new_xtranslate, m_new_ytranslate;
    KisFilterStrategy * m_new_filter;
};


#endif //_KIS_TRANSFORMATION_MASK_
