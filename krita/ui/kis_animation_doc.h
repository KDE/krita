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


#ifndef KIS_ANIMATION_DOC_H
#define KIS_ANIMATION_DOC_H

#include <krita_export.h>
#include "kis_doc2.h"
#include "kis_animation_part.h"
#include <./kranimstore/kis_animation_store.h>

#define KIS_ANIM_MIME_TYPE "application/x-krita-animation"

class KRITAUI_EXPORT KisAnimationDoc : public KisDoc2
{
    Q_OBJECT
public:
    KisAnimationDoc();
    virtual ~KisAnimationDoc();
    void frameSelectionChanged(QRect frame);
    void addKeyFrame(QRect frame);
    void addBlankFrame(QRect frame);
    KisAnimationStore* getStore();
    KisAnimation* getAnimation();

    virtual bool completeLoading(KoStore *store);
    virtual bool completeSaving(KoStore*store);

    virtual QDomDocument saveXML();
    virtual bool loadXML(const KoXmlDocument& doc, KoStore* store);

    void loadAnimationFile(KisAnimation* animation, KisAnimationStore* store);

public slots:
    void play();
    void pause();
    void stop();
    void slotFrameModified();

private:
    void preSaveAnimation();
    void updateXML();
    QString getFrameFile(int frame, int layer);
    QRect getParentFramePosition(int frame, int layer);

private:
    class KisAnimationDocPrivate;
    KisAnimationDocPrivate* const d;

signals:
    void sigFrameModified();
};

#endif // KIS_ANIMATION_DOC_H
