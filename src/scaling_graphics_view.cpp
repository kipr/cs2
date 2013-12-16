#include "scaling_graphics_view.hpp"

#include <QGraphicsItem>

ScalingGraphicsView::ScalingGraphicsView(QWidget *parent)
	: QGraphicsView(parent)
{
}

void ScalingGraphicsView::resizeEvent(QResizeEvent *event)
{
	QGraphicsView::resizeEvent(event);
  if(!scene()) return;
  QRectF bounding;
  Q_FOREACH(QGraphicsItem *const item, scene()->items()) {
    const QRectF b = item->boundingRect();
    if(bounding.width() * bounding.height() > b.width() * b.height()) continue;
    bounding = b;
  }
  setSceneRect(bounding);
  const QRectF rect = sceneRect();
  const qreal minW = qMin(width(), height());
  const qreal maxS = qMax(rect.width(), rect.height());
  QTransform transform;
  transform.scale(minW / maxS * 0.9, minW / maxS * 0.9);
  setTransform(transform);
}