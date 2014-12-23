/*
 * Copyright (c) 2013-2014 Lukáš Tvrdý <lukast.dev@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef _KIS_GMIC_PLUGIN_H_
#define _KIS_GMIC_PLUGIN_H_

#include <QVariant>

#include <kis_view_plugin.h>
#include <kis_gmic_filter_settings.h>
#include "kis_gmic_parser.h"
#include <kis_types.h>

class KoProgressUpdater;
class QTimer;
class QSize;
class QRect;
class KisGmicApplicator;
class KisGmicWidget;
class KisGmicProgressManager;

enum Activity { PREVIEWING, FILTERING };

class KisGmicPlugin : public KisViewPlugin
{
    Q_OBJECT
public:
    KisGmicPlugin(QObject *parent, const QVariantList &);
    virtual ~KisGmicPlugin();

private slots:
    // life cycle: show -> close
    void slotShowGmicDialog();
    void slotCloseGmicDialog();
    void slotRequestFinishAndClose();

    void slotPreviewGmicCommand(KisGmicFilterSetting* setting);
    void slotFilterCurrentImage(KisGmicFilterSetting* setting);
    void slotCancelOnCanvasPreview();
    void slotAcceptOnCanvasPreview();
    void slotPreviewActiveLayer();
    // miliseconds - time gmic spent filtering images
    void slotGmicFinished(int miliseconds);
    void slotGmicFailed(const QString& msg);
    void slotUpdateProgress();


private:
    void parseGmicCommandDefinitions(const QStringList &gmicDefinitionFilePaths);
    void setupDefinitionPaths();
    static KisNodeListSP createPreviewThumbnails(KisNodeListSP layers,const QSize &dstSize,const QRect &srcRect);
    void createViewportPreview(KisNodeListSP layers, KisGmicFilterSetting* setting);
    // has to be accepted or cancelled!
    void startOnCanvasPreview(KisNodeListSP layers, KisGmicFilterSetting* setting,Activity activity);
    bool checkSettingsValidity(KisNodeListSP layers, const KisGmicFilterSetting * setting);

    // TODO: refactor into responsible classes
    void showInPreviewViewport(KisPaintDeviceSP device);

    void initProgress();

private:
    KisGmicWidget * m_gmicWidget;
    KisGmicApplicator * m_gmicApplicator;
    QStringList m_definitionFilePaths;
    QString m_blacklistPath;
    QByteArray m_gmicCustomCommands;

    // progress
    KisGmicProgressManager * m_progressManager;
    Activity m_currentActivity;
    bool m_requestFinishAndClose;
};

#endif
