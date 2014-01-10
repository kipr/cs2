#ifndef _DEPTH_IMAGE_VIEW_HPP_
#define _DEPTH_IMAGE_VIEW_HPP_

#include <QWidget>
#include <QImage>
#include <QPixmap>

namespace depth
{
  class DepthImage;
}

class DepthImageView : public QWidget
{
Q_OBJECT
public:
  DepthImageView(QWidget *const parent = 0);
  ~DepthImageView();
  
  void setDepthImage(const depth::DepthImage *const image);
  const depth::DepthImage *depthImage() const;
  
  void setPleaseWait(const bool pleaseWait);
  bool isPleaseWait() const;
  
protected:
  void paintEvent(QPaintEvent *event);
  
private:
  const depth::DepthImage *_image;
  QImage _backingImage;
  QPixmap _backing;
  bool _pleaseWait;
};

#endif