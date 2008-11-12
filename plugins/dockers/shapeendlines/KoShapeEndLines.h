/*
 * KoShapeEndLines.h
 *
 *  Created on: 10 nov. 2008
 *      Author: alexia
 */

#ifndef KOSHAPEENDLINES_H_
#define KOSHAPEENDLINES_H_

#include <QListView>
#include <QDockWidget>

class KoShapeEndLines : public QDockWidget{
public:
	KoShapeEndLines(QWidget* parent = 0);
	virtual ~KoShapeEndLines();

private:
        QListView *m_quickView;
};

#endif /* KOSHAPEENDLINES_H_ */
