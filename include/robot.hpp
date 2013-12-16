#ifndef _ROBOT_HPP_
#define _ROBOT_HPP_

#include <QTime>
#include <QLineF>

class QGraphicsItem;
class QGraphicsRectItem;
class QGraphicsEllipseItem;
class QGraphicsLineItem;

class Robot
{
public:
	Robot();
	~Robot();

	void reset();
	
	void setWheelDiameter(const double &wheelDiameter);
	const double &wheelDiameter() const;
	
	void setWheelRadii(const double &wheelRadii);
	const double &wheelRadii() const;
	
	void setLeftSpeed(const double &leftSpeed);
	const double &leftSpeed() const;
	
	void setRightSpeed(const double &rightSpeed);
	const double &rightSpeed() const;
	
	void setLeftTravelDistance(double leftTravelDistance);
	double leftTravelDistance() const;
	
	void setRightTravelDistance(double rightTravelDistance);
	double rightTravelDistance() const;
	
	void setRangeLength(const double &rangeLength);
	const double &rangeLength() const;
	
	double leftRange() const;
	double frontRange() const;
	double rightRange() const;
	
	double leftReflectance() const;
	double rightReflectance() const;
  
  QPointF pos() const;

	void update();
  void restartTime();

	QList<QGraphicsItem *> robot() const;
	
private:
	void updateRangeLines();
	
	void updateReflectances();

	double reflectanceReading(double sensorX, double sensorY);

	QLineF intersectDistance(QGraphicsLineItem *item, const double &baseAngle) const;
	
	double m_wheelDiameter;
	double m_wheelRadii;
	double m_leftSpeed;
	double m_rightSpeed;
	double m_rangeLength;

	double m_leftReflectance;
	double m_rightReflectance;
	
	double m_leftTravelDistance;
	double m_rightTravelDistance;
	
	QTime m_time;
	
	QGraphicsRectItem *m_robot;
	QGraphicsEllipseItem *m_leftWheel;
	QGraphicsEllipseItem *m_rightWheel;
	
	QGraphicsLineItem *m_leftRange;
	QGraphicsLineItem *m_frontRange;
	QGraphicsLineItem *m_rightRange;
	
	QGraphicsEllipseItem *m_front;
};

#endif
