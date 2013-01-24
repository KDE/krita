/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_DOC2_P_H
#define __KIS_DOC2_P_H

#include "kundo2stack.h"
#include "kis_image.h"

class KisDoc2::UndoStack : public KUndo2Stack
{
public:
    UndoStack(KisDoc2 *doc)
        : m_doc(doc)
    {
    }

    void setIndex(int idx) {
        KisImageWSP image = this->image();
        image->requestStrokeCancellation();
        if(image->tryBarrierLock()) {
            KUndo2Stack::setIndex(idx);
            image->unlock();
        }
    }

    void undo() {
        KisImageWSP image = this->image();
        image->requestUndoDuringStroke();
        if(image->tryBarrierLock()) {
            KUndo2Stack::undo();
            image->unlock();
        }
    }

    void redo() {
        KisImageWSP image = this->image();
        if(image->tryBarrierLock()) {
            KUndo2Stack::redo();
            image->unlock();
        }
    }

private:
    KisImageWSP image() {
        KisImageWSP currentImage = m_doc->image();
        Q_ASSERT(currentImage);
        return currentImage;
    }

private:
    KisDoc2 *m_doc;
};

#endif /* __KIS_DOC2_P_H */
