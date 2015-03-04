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

class KisGmicSmallApplicator;
class KisGmicApplicator;
class KisGmicWidget;
class KisGmicProgressManager;



class KisGmicPlugin : public KisViewPlugin
{
    Q_OBJECT
    Q_ENUMS(Activity)
public:
    KisGmicPlugin(QObject *parent, const QVariantList &);
    virtual ~KisGmicPlugin();

    enum Activity { INIT, PREVIEWING, FILTERING, SMALL_PREVIEW };
    static QLatin1String valueToQString(Activity activity);


signals:
    void filteringFinished();

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
    void slotPreviewSmallWindow(KisPaintDeviceSP device);
    // miliseconds - time gmic spent filtering images
    void slotGmicFinished(bool successfully, int miliseconds = -1, const QString& msg = QString());
    void slotUpdateProgress();
    void slotPreviewReady();


private:
    void parseGmicCommandDefinitions(const QStringList &gmicDefinitionFilePaths);
    void setupDefinitionPaths();
    void createViewportPreview(KisNodeListSP layers, KisGmicFilterSetting* setting);
    // has to be accepted or cancelled!
    void startOnCanvasPreview(KisNodeListSP layers, KisGmicFilterSetting* setting, Activity activity);
    bool checkSettingsValidity(KisNodeListSP layers, const KisGmicFilterSetting * setting);

    void setActivity(Activity activity);

    void gmicFailed(const QString& msg);
    void gmicFinished(int miliseconds);

    void waitForFilterFinish();


private:
    KisGmicWidget * m_gmicWidget;
    KisGmicApplicator * m_gmicApplicator;
    KisGmicSmallApplicator * m_smallApplicator;
    QStringList m_definitionFilePaths;
    QString m_blacklistPath;
    QByteArray m_gmicCustomCommands;

    // progress
    KisGmicProgressManager * m_progressManager;
    Activity m_currentActivity;
    bool m_requestFinishAndClose;

    quint32 m_smallPreviewRequestCounter;
    quint32 m_onCanvasPreviewRequestCounter;

    bool m_filteringIsRunning;

};

#endif
