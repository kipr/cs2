#ifndef _SCALING_GRAPHICS_VIEW_HPP_
#define _SCALING_GRAPHICS_VIEW_HPP_

#include <QGraphicsView>

class ScalingGraphicsView : public QGraphicsView
{
Q_OBJECT
public:
	ScalingGraphicsView(QWidget *parent = 0);
	
protected:
	void resizeEvent(QResizeEvent *event);
};

#endif
