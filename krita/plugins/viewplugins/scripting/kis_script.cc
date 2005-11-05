/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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
#include "kis_script.h"
#include <qbuffer.h>

KisScript::KisScript(KURL url, bool execute ) : ScriptContainer(url.path()), m_url(url), m_id(url.path(), url.fileName())
{
    setInterpreterName("python");
    
    reload();
    
    if(execute)
    {
        connect(this, SIGNAL(loaded()), this, SLOT(execute()));
    }
}

KisScript::KisScript(const QString& name, QString language) : ScriptContainer(name), m_id(name, name)
{
    setInterpreterName(language);
}


KisScript::~KisScript()
{
}

void KisScript::reload()
{
    m_buffer = new QBuffer();
    m_buffer->open(IO_WriteOnly);
    KIO::TransferJob *job = KIO::get(m_url);
    connect(job,  SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(slotData(KIO::Job*, const QByteArray&)));
    connect(job,  SIGNAL(result(KIO::Job*)), this, SLOT(slotResult(KIO::Job*)));
}

void KisScript::slotData(KIO::Job* , const QByteArray& data)
{
    m_buffer->writeBlock(data.data(), data.size());
}

void KisScript::slotResult(KIO::Job* job)
{
    if (job->error())
        job->showErrorDialog();

    setCode(QString(m_buffer->buffer()));
    delete m_buffer;
    emit loaded();
}

void KisScript::execute()
{
    ScriptContainer::execute();
}

        
#include "kis_script.moc"
