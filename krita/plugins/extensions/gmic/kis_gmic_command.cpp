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

#include <QString>
#include <QFile>
#include <QTimer>
#include <QTime>

KisGmicCommand::KisGmicCommand(const QString &gmicCommandString, QSharedPointer< gmic_list<float> > images, const char * customCommands):
    m_gmicCommandString(gmicCommandString),
    m_images(images),
    m_customCommands(customCommands),
    m_firstRedo(true)
{
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

        /* process m_images using GMIC here */
        // Second step : Call G'MIC API to process input images.
        //------------------------------------------------------
        std::fprintf(stderr,"\n- 2st step : Call G'MIC interpreter.\n");

        for (unsigned int i = 0; i<m_images->_width; ++i)
        {
            std::fprintf(stderr,"   Input image %u = %ux%ux%ux%u, buffer : %p\n",i,
                        m_images->_data[i]._width,
                        m_images->_data[i]._height,
                        m_images->_data[i]._depth,
                        m_images->_data[i]._spectrum,
                        m_images->_data[i]._data);
        }



        gmic_list<char> images_names;
        try
        {
            QString gmicCmd = "-* 255 ";
            gmicCmd.append(m_gmicCommandString);
            dbgPlugins << m_gmicCommandString;
            gmic(gmicCmd.toLocal8Bit().constData(), *m_images, images_names, m_customCommands);

        }
        // Catch exception, if an error occured in the interpreter.
        catch (gmic_exception &e)
        {
            dbgPlugins << "\n- Error encountered when calling G'MIC : '%s'\n" << e.what();
            return;
        }

        // Third step : get back modified image data.
        //-------------------------------------------
        std::fprintf(stderr,"\n- 3st step : Returned %u output images.\n",m_images->_width);
        for (unsigned int i = 0; i<m_images->_width; ++i)
        {
            std::fprintf(stderr,"   Output image %u = %ux%ux%ux%u, buffer : %p\n",i,
                        m_images->_data[i]._width,
                        m_images->_data[i]._height,
                        m_images->_data[i]._depth,
                        m_images->_data[i]._spectrum,
                        m_images->_data[i]._data);
        }
    }
}

