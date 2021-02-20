/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PAINTING_ASSISTANTS_MANAGER_H
#define KIS_PAINTING_ASSISTANTS_MANAGER_H

#include <QObject>
#include <QPointer>

#include "KisView.h"
#include "kis_painting_assistants_decoration.h"
#include "KisReferenceImagesDecoration.h"

class KisViewManager;
class KisAction;
class KisActionManager;

class KisDecorationsManager : public QObject
{
    Q_OBJECT

public:
    KisDecorationsManager(KisViewManager* view);
    ~KisDecorationsManager() override;

    void setup(KisActionManager* actionManager);

    void setView(QPointer<KisView> imageView);

private Q_SLOTS:
    void updateAction();

private:
    KisPaintingAssistantsDecorationSP assistantsDecoration() const;
    KisReferenceImagesDecorationSP referenceImagesDecoration() const;

    QPointer<KisView> m_imageView;

    KisAction *m_toggleAssistant;
    KisAction *m_togglePreview;

    KisAction *m_toggleReferenceImages;
};

#endif // KIS_PAINTING_ASSISTANTS_MANAGER_H
