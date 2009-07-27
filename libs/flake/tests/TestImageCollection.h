#ifndef TESTIMAGECOLLECTION_H
#define TESTIMAGECOLLECTION_H

#include <QtTest/QtTest>

class TestImageCollection : public QObject
{
    Q_OBJECT
private slots:
    // tests
    void testGetImageImage();
    void testGetImageKUrl();
    void testGetImageStore();

    // imageData tests
    void testImageDataAsSharedData();
};

#endif /* TESTIMAGECOLLECTION_H */
