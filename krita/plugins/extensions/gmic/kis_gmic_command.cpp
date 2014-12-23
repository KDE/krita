/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_gmic_command.h"
#include <kis_debug.h>
#include <kis_processing_visitor.h>
#include <KoUpdater.h>

#include <QString>
#include <QFile>
#include <QTimer>
#include <QTime>

KisGmicCommand::KisGmicCommand(const QString &gmicCommandString, QSharedPointer< gmic_list<float> > images, const char * customCommands):
    m_gmicCommandString(gmicCommandString),
    m_images(images),
    m_customCommands(customCommands),
    m_firstRedo(true),
    m_progress(new float(-1)),
    m_cancelEvent(new bool(false))
{
}

KisGmicCommand::~KisGmicCommand()
{
    delete m_cancelEvent;
    delete m_progress;
}


void KisGmicCommand::undo()
{
    // do nothing
}

void KisGmicCommand::redo()
{
    if (m_firstRedo)
    {
        m_firstRedo = false;
        dbgPlugins << "Calling G'MIC interpreter:";
        for (unsigned int i = 0; i<m_images->_width; ++i)
        {
            dbgPlugins << "  G'MIC Input image " << i << " = " << gmicDimensionString(m_images->_data[i]) << ", buffer : " << m_images->_data[i]._data;
        }

        gmic_list<char> images_names; // unused
        QString gmicCmd = "-v - -* 255 "; // turn off verbose mode
        gmicCmd.append(m_gmicCommandString);
        dbgPlugins << "G'Mic command executed: " << gmicCmd;
        bool include_default_commands = true;

        QTime timer;
        timer.start();
        try
        {
            gmic(gmicCmd.toLocal8Bit().constData(), *m_images, images_names, m_customCommands, include_default_commands, m_progress, m_cancelEvent);
        }
        catch (gmic_exception &e)
        {
            QString message = QString::fromUtf8(e.what());
            dbgPlugins << "\n- Error encountered when calling G'MIC : " << message;

            emit gmicFailed(message);
            return;
        }

        dbgPlugins << "G'MIC returned " << m_images->_width << " output images.";
        for (unsigned int i = 0; i<m_images->_width; ++i)
        {
            dbgPlugins << "   Output image "<< i << " = " << gmicDimensionString(m_images->_data[i]) << ", buffer : " << m_images->_data[i]._data;
        }

        emit gmicFinished(timer.elapsed());
    }
}

float*KisGmicCommand::getProgress()
{
    return m_progress;
}

void KisGmicCommand::cancel()
{
    *m_cancelEvent = true;
}

QString KisGmicCommand::gmicDimensionString(const gmic_image<float>& img)
{
    return QString("%1x%2x%3x%4").arg(img._width).arg(img._height).arg(img._depth).arg(img._spectrum);
}



