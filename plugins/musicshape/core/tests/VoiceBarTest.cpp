/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include <QtTest/QtTest>

#include "VoiceBar.h"
#include "Sheet.h"
#include "Part.h"
#include "Voice.h"
#include "VoiceElement.h"
#include "Bar.h"

using namespace MusicCore;


class VoiceBarTest : public QObject
{
    Q_OBJECT
private:
    MusicCore::VoiceBar* voiceBar;
private slots:
    void init()
    {
            Sheet* sheet = new Sheet();
            Bar* bar = new Bar(sheet);
            voiceBar = new VoiceBar(bar);
    }

    void cleanup()
    {
        delete voiceBar;
    }

    void testConstruction()
    {
        //    QCOMPARE(voiceBar->voice(), voice);
        //  QCOMPARE(voiceBar->bar(), bar);
        QCOMPARE(voiceBar->elementCount(), 0);
    }

    void testAddElement()
    {
        VoiceElement *elem1 = new VoiceElement(), *elem2 = new VoiceElement();
        voiceBar->addElement(elem1);
        QCOMPARE(voiceBar->elementCount(), 1);
        QCOMPARE(voiceBar->element(0), elem1);
        
        voiceBar->addElement(elem2);
        QCOMPARE(voiceBar->elementCount(), 2);
        QCOMPARE(voiceBar->element(1), elem2);
        QCOMPARE(voiceBar->element(0), elem1);
    }

    void testInsertElement_index()
    {
        VoiceElement *elem1 = new VoiceElement(), *elem2 = new VoiceElement(), *elem3 = new VoiceElement();
        voiceBar->insertElement(elem1, 0);
        QCOMPARE(voiceBar->elementCount(), 1);
        QCOMPARE(voiceBar->element(0), elem1);
        
        voiceBar->insertElement(elem2, 0);
        QCOMPARE(voiceBar->elementCount(), 2);
        QCOMPARE(voiceBar->element(0), elem2);
        QCOMPARE(voiceBar->element(1), elem1);
        
        voiceBar->insertElement(elem3, 1);
        QCOMPARE(voiceBar->elementCount(), 3);
        QCOMPARE(voiceBar->element(0), elem2);
        QCOMPARE(voiceBar->element(1), elem3);
        QCOMPARE(voiceBar->element(2), elem1);
    }

    void testInsertElement_element()
    {
        VoiceElement *elem1 = new VoiceElement(), *elem2 = new VoiceElement(), *elem3 = new VoiceElement();
        voiceBar->addElement(elem1);
        
        voiceBar->insertElement(elem2, elem1);
        QCOMPARE(voiceBar->elementCount(), 2);
        QCOMPARE(voiceBar->element(0), elem2);
        QCOMPARE(voiceBar->element(1), elem1);
        
        voiceBar->insertElement(elem3, elem1);
        QCOMPARE(voiceBar->elementCount(), 3);
        QCOMPARE(voiceBar->element(0), elem2);
        QCOMPARE(voiceBar->element(1), elem3);
        QCOMPARE(voiceBar->element(2), elem1);
    }

    void testRemoveElement_index()
    {
        VoiceElement *elem1 = new VoiceElement(), *elem2 = new VoiceElement(), *elem3 = new VoiceElement();
        voiceBar->addElement(elem1);
        voiceBar->addElement(elem2);
        voiceBar->addElement(elem3);
        
        voiceBar->removeElement(1);
        QCOMPARE(voiceBar->elementCount(), 2);
        QCOMPARE(voiceBar->element(0), elem1);
        QCOMPARE(voiceBar->element(1), elem3);
    }

    void testRemoveElement_element()
    {
        VoiceElement *elem1 = new VoiceElement(), *elem2 = new VoiceElement(), *elem3 = new VoiceElement();
        voiceBar->addElement(elem1);
        voiceBar->addElement(elem2);
        voiceBar->addElement(elem3);
        
        voiceBar->removeElement(elem2);
        QCOMPARE(voiceBar->elementCount(), 2);
        QCOMPARE(voiceBar->element(0), elem1);
        QCOMPARE(voiceBar->element(1), elem3);
    }
};


QTEST_MAIN(VoiceBarTest)

#include <VoiceBarTest.moc>
