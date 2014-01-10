#ifndef _DEPTH_SENSOR_WIDGET_HPP_
#define _DEPTH_SENSOR_WIDGET_HPP_

#include <QWidget>

namespace Ui
{
  class DepthSensorWidget;
}

class DepthSensorWidget : public QWidget
{
Q_OBJECT
public:
  DepthSensorWidget(QWidget *const parent = 0);
  ~DepthSensorWidget();
  
private slots:
  void update();
  
private:
  Ui::DepthSensorWidget *ui;
};

#endif