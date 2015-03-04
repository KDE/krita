/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
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
#include <kis_view_plugin.h>


class KisAction;
class KisMacro;
class KisViewManager;
class KUrl;

class BigBrotherPlugin : public KisViewPlugin
{
    Q_OBJECT
public:
    BigBrotherPlugin(QObject *parent, const QVariantList &);
    virtual ~BigBrotherPlugin();

private Q_SLOTS:

    void slotOpenPlay();
    void slotOpenEdit();
    void slotStartRecordingMacro();
    void slotStopRecordingMacro();

private:
    void saveMacro(const KisMacro* macro, const KUrl& url);
    KisMacro* openMacro(KUrl* url = 0);
private:

    KisViewManager * m_view;
    KisMacro * m_recorder;
    KisAction* m_startRecordingMacroAction;
    KisAction* m_stopRecordingMacroAction;

};

#endif // bigbrotherPlugin_H
