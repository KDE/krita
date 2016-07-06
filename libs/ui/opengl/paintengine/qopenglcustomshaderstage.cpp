/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qopenglcustomshaderstage_p.h"
#include "qopenglengineshadermanager_p.h"
#include "qopenglpaintengine_p.h"
#include <private/qpainter_p.h>

QT_BEGIN_NAMESPACE

class QOpenGLCustomShaderStagePrivate
{
public:
    QOpenGLCustomShaderStagePrivate() :
        m_manager(0) {}

    QPointer<QOpenGLEngineShaderManager> m_manager;
    QByteArray              m_source;
};




QOpenGLCustomShaderStage::QOpenGLCustomShaderStage()
    : d_ptr(new QOpenGLCustomShaderStagePrivate)
{
}

QOpenGLCustomShaderStage::~QOpenGLCustomShaderStage()
{
    Q_D(QOpenGLCustomShaderStage);
    if (d->m_manager) {
        d->m_manager->removeCustomStage();
        d->m_manager->sharedShaders->cleanupCustomStage(this);
    }
    delete d_ptr;
}

void QOpenGLCustomShaderStage::setUniformsDirty()
{
    Q_D(QOpenGLCustomShaderStage);
    if (d->m_manager)
        d->m_manager->setDirty(); // ### Probably a bit overkill!
}

bool QOpenGLCustomShaderStage::setOnPainter(QPainter* p)
{
    Q_D(QOpenGLCustomShaderStage);
    if (p->paintEngine()->type() != QPaintEngine::OpenGL2) {
        qWarning("QOpenGLCustomShaderStage::setOnPainter() - paint engine not OpenGL2");
        return false;
    }
    if (d->m_manager)
        qWarning("Custom shader is already set on a painter");

    QOpenGL2PaintEngineEx *engine = static_cast<QOpenGL2PaintEngineEx*>(p->paintEngine());
    d->m_manager = QOpenGL2PaintEngineExPrivate::shaderManagerForEngine(engine);
    Q_ASSERT(d->m_manager);

    d->m_manager->setCustomStage(this);
    return true;
}

void QOpenGLCustomShaderStage::removeFromPainter(QPainter* p)
{
    Q_D(QOpenGLCustomShaderStage);
    if (p->paintEngine()->type() != QPaintEngine::OpenGL2)
        return;

    QOpenGL2PaintEngineEx *engine = static_cast<QOpenGL2PaintEngineEx*>(p->paintEngine());
    d->m_manager = QOpenGL2PaintEngineExPrivate::shaderManagerForEngine(engine);
    Q_ASSERT(d->m_manager);

    // Just set the stage to null, don't call removeCustomStage().
    // This should leave the program in a compiled/linked state
    // if the next custom shader stage is this one again.
    d->m_manager->setCustomStage(0);
    d->m_manager = 0;
}

QByteArray QOpenGLCustomShaderStage::source() const
{
    Q_D(const QOpenGLCustomShaderStage);
    return d->m_source;
}

// Called by the shader manager if another custom shader is attached or
// the manager is deleted
void QOpenGLCustomShaderStage::setInactive()
{
    Q_D(QOpenGLCustomShaderStage);
    d->m_manager = 0;
}

void QOpenGLCustomShaderStage::setSource(const QByteArray& s)
{
    Q_D(QOpenGLCustomShaderStage);
    d->m_source = s;
}

QT_END_NAMESPACE
