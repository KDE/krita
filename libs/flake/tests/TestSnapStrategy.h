/*
    Copyright (C) 2012 <hanna.et.scott@gmail.com> 

    SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef TESTSNAPSTRATEGY_H
#define TESTSNAPSTRATEGY_H

#include <QObject>

class TestSnapStrategy : public QObject
{
    Q_OBJECT
  private Q_SLOTS:
    //tests
    /**
     * This method is for testing the function snap in OrthogonalSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */
    void testOrthogonalSnap();
    /**
     * This method is for testing the function snap in NodeSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */
    void testNodeSnap();
    /**
     * This method is for testing the function snap in ExtensionSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */
    void testExtensionSnap();
    /**
     * This method is for testing the function snap in IntersectionSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */
    void testIntersectionSnap();
    /**
     * This method is for testing the function snap in GridSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */
    void testGridSnap();
        /**
     * This method is for testing the function snap in BoundingBoxSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */
    void testBoundingBoxSnap();
        /**
     * This method is for testing the function snap in LineGuideSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */
    void testLineGuideSnap();

    /**
     * This method is for testing the function decoration in OrthogonalSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */
    void testOrhogonalDecoration();
    /**
     * This method is for testing the function decoration in NodeSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */
    void testNodeDecoration();
    /**
     * This method is for testing the function decoration in ExtensionSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */
    void testExtensionDecoration();
    /**
     * This method is for testing the function decoration in IntersectionSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */
    void testIntersectionDecoration();
    /**
     * This method is for testing the function decoration GridSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */
    void testGridDecoration();
    /**
     * This method is for testing the function decoration in BoundingBoxSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */
    void testBoundingBoxDecoration();
    /**
     * This method is for testing the function decoration in LineGuideSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */
    void testLineGuideDecoration();
    /**
     * This method is for testing the function squareDistance in KoSnapStrategy
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */    
    void testSquareDistance();
    /**
     * This method is for testing the function scalarProduct in KoSnapStrategy
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */   
    void testScalarProduct();  
    /**
     * This method is for testing the function snapToExtension in ExtensionSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */   
    void testSnapToExtension();
    /**
     * This method is for testing the function project in ExtensionSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */   
    void testProject();
    /**
     * This method is for testing the function extensionDirection in ExtensionSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */   
    void testExtensionDirection();
    /**
     * This method is for testing the function squareDistanceToLine in BoundingBoxSnapStrategy - function is located in KoSnapStrategy.h
     * 
     * @param 
     * @return 
     * @see KoSnapStrategy.h
     */   
    void testSquareDistanceToLine();
        
};

#endif // TESTSNAPSTRATEGY_H
