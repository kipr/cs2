#define _USE_MATH_DEFINES

#include "robot.hpp"

#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QPen>
#include <QDebug>
#include "board_file.hpp"

#include <cmath>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

#include "kovan_spiral-inl.hpp"

//TODO: load these
static const double robotRad = 10.0;
static const double boardMaxX = 243.205 - robotRad;
static const double boardMaxY = 243.205 - robotRad;
static const double boardMinX = 0.0 + robotRad;
static const double boardMinY = 0.0 + robotRad;

class RobotBase : public QGraphicsRectItem
{
public:
	RobotBase(double x, double y, double w, double h)
		: QGraphicsRectItem(x, y, w, h),
		m_grabbed(false),
		m_turnEvent(false)
	{
		setFlag(QGraphicsItem::ItemIsFocusable);
	}
	
protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event)
	{
		m_grabbed = true;
		
	}
	
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event)
	{
		if(!m_grabbed) return;
		if(!m_turnEvent) {
			setPos(event->scenePos());
		} else {
			QLineF line(pos(), event->scenePos());
			setRotation(360 - line.angle());
		}
	}
	
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
	{
		m_grabbed = false;
	}
	
	virtual void keyPressEvent(QKeyEvent *event)
	{
		m_turnEvent = event->modifiers() & Qt::ShiftModifier;
		if(m_turnEvent) event->accept();
		else event->ignore();
	}
	
	virtual void keyReleaseEvent(QKeyEvent *event)
	{
		m_turnEvent = event->modifiers() & Qt::ShiftModifier;
		if(m_turnEvent) event->accept();
		else event->ignore();
	}
	
private:
	bool m_grabbed;
	bool m_turnEvent;
};

Robot::Robot()
	: m_wheelDiameter(16.0),
	m_wheelRadii(3.0),
	m_leftSpeed(0.0),
	m_rightSpeed(0.0),
	m_rangeLength(70.0),
	m_leftTravelDistance(0.0),
	m_rightTravelDistance(0.0),
	m_robot(new RobotBase(-m_wheelDiameter / 2.0, -m_wheelDiameter / 2.0, m_wheelDiameter, m_wheelDiameter)),
	m_leftWheel(new QGraphicsEllipseItem(-m_wheelRadii, -m_wheelDiameter / 2.0 - m_wheelRadii, m_wheelRadii * 2, m_wheelRadii)),
	m_rightWheel(new QGraphicsEllipseItem(-m_wheelRadii, m_wheelDiameter / 2.0, m_wheelRadii * 2, m_wheelRadii)),
	m_leftRange(new QGraphicsLineItem()),
	m_frontRange(new QGraphicsLineItem()),
	m_rightRange(new QGraphicsLineItem()),
	m_front(new QGraphicsEllipseItem(m_wheelDiameter / 8.0, -m_wheelDiameter / 10.0, m_wheelDiameter / 5.0, m_wheelDiameter / 5.0))
{
	m_robot->setData(0, BoardFile::Fake);
	m_front->setParentItem(m_robot);
	m_leftWheel->setData(0, BoardFile::Fake);
	m_rightWheel->setData(0, BoardFile::Fake);
	m_robot->setBrush(Qt::lightGray);
	
	m_leftWheel->setParentItem(m_robot);
	m_rightWheel->setParentItem(m_robot);
	
	m_leftWheel->setBrush(Qt::darkGray);
	m_rightWheel->setBrush(Qt::darkGray);
	
	updateRangeLines();
	
	QPen rangePen(Qt::red, 0, Qt::DotLine);
	m_leftRange->setPen(rangePen);
	m_frontRange->setPen(rangePen);
	m_rightRange->setPen(rangePen);
	
	m_leftRange->setZValue(-0.1);
	m_frontRange->setZValue(-0.1);
	m_rightRange->setZValue(-0.1);

	this->reset();

	m_time.start();
}

void Robot::reset()
{
	m_robot->setX(15.0);
	m_robot->setY(15.0);
}

Robot::~Robot()
{
	delete m_robot;
	delete m_leftRange;
	delete m_frontRange;
	delete m_rightRange;
	// delete m_front;
}

void Robot::setWheelDiameter(const double &wheelDiameter)
{
	m_wheelDiameter = wheelDiameter;
}

const double &Robot::wheelDiameter() const
{
	return m_wheelDiameter;
}

void Robot::setWheelRadii(const double &wheelRadii)
{
	m_wheelRadii = wheelRadii;
}

const double &Robot::wheelRadii() const
{
	return m_wheelRadii;
}

void Robot::setLeftSpeed(const double &leftSpeed)
{
	update();
	m_leftSpeed = leftSpeed;
}

const double &Robot::leftSpeed() const
{
	return m_leftSpeed;
}

void Robot::setRightSpeed(const double &rightSpeed)
{
	update();
	m_rightSpeed = rightSpeed;
}

const double &Robot::rightSpeed() const
{
	return m_rightSpeed;
}

void Robot::setLeftTravelDistance(double leftTravelDistance)
{
	m_leftTravelDistance = leftTravelDistance;
}

double Robot::leftTravelDistance() const
{
	return m_leftTravelDistance;
}

void Robot::setRightTravelDistance(double rightTravelDistance)
{
	m_rightTravelDistance = rightTravelDistance;
}

double Robot::rightTravelDistance() const
{
	return m_rightTravelDistance;
}

void Robot::setRangeLength(const double &rangeLength)
{
	m_rangeLength = rangeLength;
	updateRangeLines();
}

const double &Robot::rangeLength() const
{
	return m_rangeLength;
}

double Robot::leftRange() const
{
	return m_leftRange->line().length();
}

double Robot::frontRange() const
{
	return m_frontRange->line().length();
}

double Robot::rightRange() const
{
	return m_rightRange->line().length();
}


double Robot::leftReflectance() const
{
	return m_leftReflectance;
}

double Robot::rightReflectance() const
{
	return m_rightReflectance;
}


void Robot::update()
{
	int milli = m_time.elapsed();
	double sec = milli / 1000.0;
	
	const double theta = m_robot->rotation() / 180.0 * M_PI;
	
	double dl = sec * m_leftSpeed * m_wheelRadii * 2.0 * M_PI;
	double dr = sec * m_rightSpeed * m_wheelRadii * 2.0 * M_PI;
	double dd = (dl + dr) / 2.0;
	m_robot->setRotation((theta + (dr - dl) / m_wheelDiameter) * 180.0 / M_PI);
	m_leftTravelDistance += dl;
	m_rightTravelDistance += dr;

	double newX = m_robot->x() + cos(theta) * dd;
	double newY = m_robot->y() + sin(theta) * dd;

	m_robot->setX(newX);
	m_robot->setY(newY);


	updateRangeLines();
	updateReflectances();
	

	m_time.restart();
}

void Robot::restartTime()
{
  m_time.restart();
}

QPointF Robot::pos() const
{
  return m_robot->pos();
}

QList<QGraphicsItem *> Robot::robot() const
{
	return QList<QGraphicsItem *>() << m_robot << m_leftRange << m_frontRange << m_rightRange;
}

void Robot::updateRangeLines()
{
	m_leftRange->setLine(intersectDistance(m_leftRange, -45.0));
	m_frontRange->setLine(intersectDistance(m_frontRange, 0.0));
	m_rightRange->setLine(intersectDistance(m_rightRange, 45.0));
}



template<typename T>
T sign(T t)
{
	if(t < 0) return -1;
	else if(t > 0) return 1;
	return 0;
}

void Robot::updateReflectances()
{
	const double sensor_dist = 10;

	// left reflectance update
	const double radLeft = (m_robot->rotation() - 30) / 180.0 * M_PI;
	const double leftSensorX = m_robot->x() + cos(radLeft) * sensor_dist;
	const double leftSensorY = m_robot->y() + sin(radLeft) * sensor_dist;
	m_leftReflectance = reflectanceReading(leftSensorX, leftSensorY);

	// right reflectance update
	const double radRight = (m_robot->rotation() + 30) / 180.0 * M_PI;
	const double rightSensorX = m_robot->x() + cos(radRight) * sensor_dist;
	const double rightSensorY = m_robot->y() + sin(radRight) * sensor_dist;
	m_rightReflectance = reflectanceReading(rightSensorX, rightSensorY);
}

double Robot::reflectanceReading(double sensorX, double sensorY)
{
	double result = 0.0;
	double weight = 1.0 / num_pts;
	double spiral_scale = 2.0;

	QGraphicsScene *scene = m_leftRange->scene();

	for (int i = 0; i < num_pts; i++){
		double spiralX = spiral_xs[i]*spiral_scale + sensorX;
		double spiralY = spiral_ys[i]*spiral_scale + sensorY;

		QRectF r(QPoint(spiralX, spiralY), QSize(1,1));

		QList<QGraphicsItem *> items = scene->items(r, Qt::IntersectsItemBoundingRect, Qt::AscendingOrder);
		foreach(QGraphicsItem *t, items) {
			if(t->data(0) == BoardFile::Tape) {
				result += weight;
				continue;
			}
		}
	}

	return result;
}

QLineF Robot::intersectDistance(QGraphicsLineItem *item, const double &baseAngle) const
{
	QGraphicsScene *scene = item->scene();
	if(!scene) return QLineF(0, 0, 0, 0);
	
	const double rad = (m_robot->rotation() + baseAngle) / 180.0 * M_PI;
	QLineF line(m_robot->pos(), m_robot->pos() + m_rangeLength *
		QPointF(cos(rad), sin(rad)));
	
	const QLineF unit = line.unitVector();
	// qDebug() << unit.dx();
	double xs = unit.dx();
	double ys = unit.dy();
	if(xs == 0.0) xs = 0.0001;
	if(ys == 0.0) ys = 0.0001;
	for(double i = 0; i < m_rangeLength; i += 1.0) {
		QRectF r(m_robot->x() + i * xs, m_robot->y() + i * ys, xs, ys);
		QList<QGraphicsItem *> items = scene->items(r, Qt::IntersectsItemBoundingRect, Qt::AscendingOrder);
		foreach(QGraphicsItem *t, items) {
			if(t->data(0) == BoardFile::Real) {
				line.setLength(i);
				return line;
			}
		}
	}
	
	return line;
}
