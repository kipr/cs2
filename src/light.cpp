#include "light.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QRadialGradient>
#include <QPen>

Light::Light()
	: QGraphicsEllipseItem(-5, -5, 10, 10),
	m_grabbed(false),
	m_gradient(new QGraphicsEllipseItem(-50, -50, 100, 100, this)),
	m_on(true)
{
	m_gradient->setPen(QPen(Qt::transparent));
	
	setPen(QPen(Qt::black));
	mouseDoubleClickEvent(0);
	this->reset();
}

Light::~Light()
{
}

bool Light::isOn() const
{
	return m_on;
}

void Light::reset()
{
	setX(0.0);
	setY(0.0);
	setOn(false);
}

void Light::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	m_grabbed = true;
}

void Light::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if(!m_grabbed) return;
	setPos(event->scenePos());
}

void Light::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	m_grabbed = false;
}

void Light::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	this->setOn(!m_on);
}

void Light::setOn(bool on)
{
	m_on = on;
	QRadialGradient gradient(0, 0, 50);
	QColor color = m_on ? QColor(255, 215, 0, 127) : QColor(50, 50, 50, 100);
	gradient.setColorAt(0.0, color);
	gradient.setColorAt(1.0, Qt::transparent);
	m_gradient->setBrush(gradient);
}
