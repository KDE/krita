/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_random_generator_test.h"

#include <qtest_kde.h>
#include "kis_random_generator.h"

#include <math.h>

void KisRandomGeneratorTest::testConstantness(int seed)
{
    KisRandomGenerator rand1( seed );
    KisRandomGenerator rand2( seed );
    for( int y = 0; y < 100; y++)
    {
        for( int x = 0; x < 100; x++)
        {
            QVERIFY( rand1.randomAt(x,y) == rand2.randomAt(x,y));
            QVERIFY( rand1.randomAt(x,y) >= 0 );
            QVERIFY( rand1.randomAt(x,y) <= RAND_MAX );
            QVERIFY( fabs(rand1.doubleRandomAt(x,y) - rand2.doubleRandomAt(x,y)) < 1e-5  );
        }
    }
}


void KisRandomGeneratorTest::testConstantness()
{
    testConstantness(50);
    testConstantness(75);
    testConstantness(175);
    testConstantness(3000);
    testConstantness(5000);
    testConstantness(6050);
}

#include <iostream>

void KisRandomGeneratorTest::testEvolution()
{
    int counter = 0;
    
    KisRandomGenerator randg( 10000 );
    for( int y = 2; y < 1000; y++)
    {
        for( int x = 2; x < 1000; x++)
        {
            int number = randg.randomAt(x,y);
            double dnumber = randg.doubleRandomAt(x,y);
            
            for(int i = -2; i < 3; i++)
            {
                for(int j = -2; j < 3; j++)
                {
                    if( i != 0 && j != 0)
                    {
                        QVERIFY( number != randg.randomAt( x + i, y + j) );
                        double dnumber2 = randg.doubleRandomAt( x + i, y + j);
                        if (!(fabs( dnumber - dnumber2 ) > 0.001 )) {
                            //qDebug() << dnumber << ", " << dnumber2 << ", " << fabs( dnumber - dnumber2 );
                            counter++;
                        }
                        //QVERIFY( fabs( dnumber - dnumber2 ) > 0.000000001 );
                    }
                }
            }
        }
    }
    qDebug() << counter;
}

QTEST_KDEMAIN(KisRandomGeneratorTest, GUI)
#include "kis_random_generator_test.moc"
