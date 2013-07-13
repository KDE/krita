/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version..
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_ANIMATION_PART_H
#define KIS_ANIMATION_PART_H

#include "kis_part2.h"
#include "kis_animation_doc.h"
#include "kis_animation.h"

class KRITAUI_EXPORT KisAnimationPart : public KisPart2
{
    Q_OBJECT

public:
    KisAnimationPart(QObject* parent = 0);

    virtual ~KisAnimationPart();

    KisAnimation* animation() const { return m_animation; }
    void setAnimation(KisAnimation *animation);

protected:
    //Reimplemented
    QList<KoPart::CustomDocumentWidgetItem> createCustomDocumentWidgets(QWidget *parent);

private:
    KisAnimation* m_animation;
};

#endif // KIS_ANIMATION_PART_H
