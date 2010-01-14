/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _BIGBROTHER_H_
#define _BIGBROTHER_H_

#include <QVariant>

#include <kparts/plugin.h>

class QAction;
class KisMacro;
class KisView2;
class KUrl;

/**
 * Template of view plugin
 */
class BigBrotherPlugin : public KParts::Plugin
{
    Q_OBJECT
public:
    BigBrotherPlugin(QObject *parent, const QVariantList &);
    virtual ~BigBrotherPlugin();

private slots:

    void slotSave();
    void slotOpenPlay();
    void slotOpenEdit();
    void slotStartRecordingMacro();
    void slotStopRecordingMacro();

private:
    void saveMacro(const KisMacro* macro, const KUrl& url);
    KisMacro* openMacro(KUrl* url = 0);
private:

    KisView2 * m_view;
    KisMacro * m_recorder;
    KAction* m_startRecordingMacroAction;
    KAction* m_stopRecordingMacroAction;

};

#endif // bigbrotherPlugin_H
