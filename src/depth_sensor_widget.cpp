#include "depth_sensor_widget.hpp"
#include "ui_depth_sensor_widget.h"

#include <kovan/depth_driver.hpp>
#include <QTimer>
#include <QDebug>

using namespace depth;

DepthSensorWidget::DepthSensorWidget(QWidget *const parent)
  : QWidget(parent)
  , ui(new Ui::DepthSensorWidget)
{
  ui->setupUi(this);
  
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), SLOT(update()));
  timer->start(50); // 20 Hz
}

DepthSensorWidget::~DepthSensorWidget()
{
  try {
    DepthDriver::instance().close();
  } catch(...) {}
  delete ui;
}

void DepthSensorWidget::update()
{
  DepthDriver &driver = DepthDriver::instance();
  if(!driver.isOpen()) {
    try {
      driver.open();
    } catch(...) {}
    if(!driver.isOpen()) {
      qDebug() << "Not open yet...";
      ui->view->setPleaseWait(false);
      ui->view->setDepthImage(0);
      return;
    }
  }
  
  ui->view->setDepthImage(driver.depthImage());
}
