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

#ifndef __KIS_STROKE_STRATEGY_H
#define __KIS_STROKE_STRATEGY_H

#include <QString>
#include "kis_dab_processing_strategy.h"

class KisStrokeStrategy;


class KRITAIMAGE_EXPORT KisStrokeStrategy
{
public:
    KisStrokeStrategy(QString id = QString(), QString name = QString());
    virtual ~KisStrokeStrategy();

    virtual KisDabProcessingStrategy* createInitStrategy();
    virtual KisDabProcessingStrategy* createFinishStrategy();
    virtual KisDabProcessingStrategy* createCancelStrategy();
    virtual KisDabProcessingStrategy* createDabStrategy();

    bool isExclusive() const;
    bool needsIndirectPainting() const;

    QString id() const;
    QString name() const;

protected:
    // you are not supposed to change these parameters
    // in places other than a constructor

    void setExclusive(bool value);
    void setNeedsIndirectPainting(bool value);

private:
    bool m_exclusive;
    bool m_needsIndirectPainting;

    QString m_id;
    QString m_name;
};

#endif /* __KIS_STROKE_STRATEGY_H */
