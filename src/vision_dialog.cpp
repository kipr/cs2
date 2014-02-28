#include "vision_dialog.hpp"
#include "ui_vision_dialog.h"

#include "vision_config_model.hpp"
#include "vision_channel_model.hpp"
#include "input_dialog.hpp"

#include <kovan/camera.hpp>

#include <QItemSelection>
#include <QFileInfo>
#include <QItemDelegate>
#include <QPainter>
#include <QDir>
#include <QDebug>

class ConfigItemDelegate : public QItemDelegate
{
public:
	ConfigItemDelegate(VisionDialog *parent = 0)
		: QItemDelegate(parent),
		m_star(QIcon(":/icons/star.png").pixmap(16, 16))
	{
	}
	
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QItemDelegate::paint(painter, option, index);
		VisionDialog *w = qobject_cast<VisionDialog *>(parent());
		if(!w->isDefaultConfig(index)) return;
		const QPoint right = option.rect.topRight();
		painter->drawPixmap(right.x() - 24, right.y() + option.rect.height() / 2 - 8,
			16, 16, m_star);
	}
	
	QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
	{
		return QSize(0, 22);
	}
	
private:
	QPixmap m_star;
};

class ChannelItemDelegate : public QItemDelegate
{
public:
	ChannelItemDelegate(VisionChannelModel *model, QObject *parent = 0)
		: QItemDelegate(parent),
		m_model(model),
		m_hsv(QIcon(":/icons/color_wheel.png").pixmap(16, 16)),
		m_qr(QIcon(":/icons/qr.png").pixmap(16, 16))
	{
	}
	
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QItemDelegate::paint(painter, option, index);

		const QString &type = m_model->channelType(index);

		QPixmap pixmap;
		if(type == CAMERA_CHANNEL_TYPE_HSV_KEY) pixmap = m_hsv;
		else if(type == CAMERA_CHANNEL_TYPE_QR_KEY) pixmap = m_qr;

		const QPoint right = option.rect.topRight();
		painter->drawPixmap(right.x() - 24, right.y() + option.rect.height() / 2 - 8,
			16, 16, pixmap);
	}
	
private:
	VisionChannelModel *m_model;
	QPixmap m_hsv;
	QPixmap m_qr;
};

/* Keep? */
VisionDialog::VisionDialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::VisionDialog)
	, m_configModel(new VisionConfigModel(this))
	, m_channelModel(new VisionChannelModel(this))
	, m_device(new Camera::Device(new Camera::UsbInputProvider))
{
	ui->setupUi(this);
	
	ui->configsList->setModel(m_configModel);
	ui->configsList->setRootIndex(m_configModel->index(m_configModel->rootPath()));
	ui->configsList->setItemDelegate(new ConfigItemDelegate(this));
	
	ui->channelsList->setModel(m_channelModel);
	ui->channelsList->setItemDelegate(new ChannelItemDelegate(m_channelModel, this));
	
	connect(&m_cameraTimer, SIGNAL(timeout()), SLOT(updateCamera()));
	m_cameraTimer.start(100);
	
  /* Need this? */
	Config startConfig;
	startConfig.beginGroup(CAMERA_GROUP);
	startConfig.setValue(CAMERA_NUM_CHANNELS_KEY, 1);
	startConfig.beginGroup((QString(CAMERA_CHANNEL_GROUP_PREFIX) + "0").toStdString());
	startConfig.setValue(CAMERA_CHANNEL_TYPE_KEY, CAMERA_CHANNEL_TYPE_HSV_KEY);
	startConfig.clearGroup();
	m_device->setConfig(startConfig);
	
	connect(ui->_th, SIGNAL(textChanged(QString)), SLOT(manualEntry(QString)));
	connect(ui->_ts, SIGNAL(textChanged(QString)), SLOT(manualEntry(QString)));
	connect(ui->_tv, SIGNAL(textChanged(QString)), SLOT(manualEntry(QString)));
	connect(ui->_bh, SIGNAL(textChanged(QString)), SLOT(manualEntry(QString)));
	connect(ui->_bs, SIGNAL(textChanged(QString)), SLOT(manualEntry(QString)));
	connect(ui->_bv, SIGNAL(textChanged(QString)), SLOT(manualEntry(QString)));
	
	connect(ui->visual, SIGNAL(minChanged(QColor)), SLOT(visualChanged()));
	connect(ui->visual, SIGNAL(maxChanged(QColor)), SLOT(visualChanged()));
	
	connect(ui->cv, SIGNAL(pressed(int, int)), SLOT(imagePressed(int, int)));
	
	connect(ui->channelsList->selectionModel(),
		SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
		SLOT(currentChannelChanged(const QModelIndex &, const QModelIndex &)));
	
	connect(ui->configsList->selectionModel(),
		SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
		SLOT(currentConfigChanged(const QModelIndex &, const QModelIndex &)));
		
	m_defaultConfigPath = m_configModel->filePath(m_configModel->defaultConfiguration());
}

VisionDialog::~VisionDialog()
{
	delete m_device;
}

int VisionDialog::exec()
{
	if(!m_device->open()) return QDialog::Rejected;
	const int ret = QDialog::exec();
	m_device->close();
	
	const QModelIndex configIndex = ui->configsList->selectionModel()->currentIndex();
	if(configIndex.isValid()) currentConfigChanged(QModelIndex(), configIndex);
	
	return ret;
}

void VisionDialog::setDefault(const QModelIndex &index)
{
  if(!index.isValid()) return;
  const QModelIndex oldDefault = m_configModel->index(m_defaultConfigPath);
  m_defaultConfigPath = m_configModel->filePath(index);
  ui->defaultConfig->setEnabled(false);
  Camera::ConfigPath::setDefaultConfigPath(QFileInfo(m_defaultConfigPath).baseName().toStdString());
  ui->configsList->update(index);
  if(oldDefault.isValid()) ui->configsList->update(oldDefault);
}

bool VisionDialog::isDefaultConfig(const QModelIndex &index) const
{
	return m_configModel->filePath(index) == m_defaultConfigPath;
}

void VisionDialog::on_addConfig_clicked()
{  
  // Get new configuration name from user
  InputDialog input(this);
  input.setKey(tr("New Configuration Name"));
  input.setEmptyAllowed(false);
  if(input.exec() != QDialog::Accepted) return;
  
  // Save new configuration
  std::string savePath = Camera::ConfigPath::path(input.value().toStdString());
  Config blank;
  if(!blank.save(savePath)) return;
  
  setDefault(m_configModel->index(QString::fromStdString(savePath)));
}

void VisionDialog::on_removeConfig_clicked()
{  
  const QModelIndex index = ui->configsList->selectionModel()->currentIndex();
  if(!index.isValid()) return;
  if(!QFile::remove(m_configModel->filePath(index))) qDebug() << "Failed to remove file!";
}

void VisionDialog::on_duplicateConfig_clicked()
{  
  const QModelIndex index = ui->configsList->selectionModel()->currentIndex();
  if(!index.isValid()) return;
  
  InputDialog input(this);
  input.setKey(tr("Duplicate Configuration Name"));
  input.setEmptyAllowed(false);
  if(input.exec() != QDialog::Accepted) return;
  
  std::string savePath = Camera::ConfigPath::path(input.value().toStdString());
  Config *config = Config::load(m_configModel->filePath(index).toStdString());
  if(!config->save(savePath)) {
    qWarning() << "Error saving" << QString::fromStdString(savePath);
    return;
  }
}

void VisionDialog::on_defaultConfig_clicked()
{
  const QModelIndex index = ui->configsList->selectionModel()->currentIndex();
  if(!index.isValid()) return;
  setDefault(index);
}

void VisionDialog::on_addChannel_clicked()
{
  m_channelModel->addChannel(CAMERA_CHANNEL_TYPE_HSV_KEY);
}

// TODO: this crashes if removing the first row
void VisionDialog::on_removeChannel_clicked()
{
  const QModelIndex index = ui->configsList->selectionModel()->currentIndex();
  if(!index.isValid()) return;
  const int row = index.row();
  m_channelModel->removeChannel(row);
}

void VisionDialog::on_up_clicked()
{
  const QModelIndex index = ui->configsList->selectionModel()->currentIndex();
  if(!index.isValid()) return;
  const int row = index.row();
  if(row == 0) return;
  
  m_channelModel->swapChannels(row, row - 1);
}

void VisionDialog::on_down_clicked()
{
  const QModelIndex index = ui->configsList->selectionModel()->currentIndex();
  if(!index.isValid()) return;
  const int row = index.row();
  if(row >= m_configModel->rowCount() - 1) return;
  
  m_channelModel->swapChannels(row, row + 1);
}

void VisionDialog::updateCamera()
{
	if(!m_device->update()) {
		qWarning() << "Lost camera";
		ui->cv->setInvalid(true);
		return;
	}

	cv::Mat image = m_device->rawImage();
	
	if(!ui->hsv->isEnabled()) {
		ui->cv->updateImage(image);
		return;
	}

	const Camera::ObjectVector *objs = m_device->channels()[0]->objects();
	if(!objs) {
		qWarning() << "Objects invalid";
		ui->cv->setInvalid(true);
		return;
	}
  
  const int maxBlobNums = ui->numBlobs->value();
  if(!ui->showBoundingBox->isChecked() && maxBlobNums <= 0) {
    ui->cv->updateImage(image);
    return;
  }
	
  int blobNum = 0;
	for(Camera::ObjectVector::const_iterator it = objs->begin(); it != objs->end(); ++it) {
		const Camera::Object &obj = *it;
    
    // Draw bounding box if necessary
    if(ui->showBoundingBox->isChecked())
      cv::rectangle(image, cv::Rect(obj.boundingBox().x(), obj.boundingBox().y(),
        obj.boundingBox().width(), obj.boundingBox().height()),
        cv::Scalar(255, 0, 0), 2);
      
    // Draw blob number if max not exceeded
    if(blobNum < maxBlobNums) {
      int fontface = cv::FONT_HERSHEY_SIMPLEX;
      double scale = 1.5;
      int thickness = 2;
      int baseline = 0;
      
      cv::Size textSize = cv::getTextSize(std::to_string(blobNum), fontface, scale, thickness, &baseline);
      cv::Point bl(obj.boundingBox().x() + (obj.boundingBox().width() - textSize.width) / 2,
        obj.boundingBox().y() + (obj.boundingBox().height() + textSize.height) / 2);
      cv::rectangle(image, bl, bl + cv::Point(textSize.width, -textSize.height), cv::Scalar(0, 0, 0), CV_FILLED);
      cv::putText(image, std::to_string(blobNum), bl, fontface, scale, cv::Scalar(255, 255, 255), thickness);
      
      ++blobNum;
    }
	}
	
	ui->cv->updateImage(image);
}

void VisionDialog::visualChanged()
{
	if(!ui->hsv->isEnabled()) return;
	
	const QColor &max = ui->visual->max();
	const QColor &min = ui->visual->min();
	
	m_hsvConfig.setValue("th", max.hue() / 2);
	m_hsvConfig.setValue("ts", max.saturation());
	m_hsvConfig.setValue("tv", max.value());
	m_hsvConfig.setValue("bh", min.hue() / 2);
	m_hsvConfig.setValue("bs", min.saturation());
	m_hsvConfig.setValue("bv", min.value());
	
	refreshHsv();
}

void VisionDialog::manualEntry(const QString &number)
{
	QObject *from = sender();
	if(!from || !ui->hsv->isEnabled()) return;
	
	int num = number.toInt();
	if(num < 0) num = 0;
	
	if(from == ui->_th) {
		m_hsvConfig.setValue("th", qMin(num, 359) >> 1);
	} else if(from == ui->_ts) {
		m_hsvConfig.setValue("ts", qMin(num, 255));
	} else if(from == ui->_tv) {
		m_hsvConfig.setValue("tv", qMin(num, 255));
	} else if(from == ui->_bh) {
		m_hsvConfig.setValue("bh", qMin(num, 359) >> 1);
	} else if(from == ui->_bs) {
		m_hsvConfig.setValue("bs", qMin(num, 255));
	} else if(from == ui->_bv) {
		m_hsvConfig.setValue("bv", qMin(num, 255));
	}
	
	refreshHsv();
}

void VisionDialog::imagePressed(const int x, const int y)
{
	if(!ui->hsv->isEnabled()) return;
	
	cv::Mat image = m_device->rawImage();
	cv::Vec3b data = image.at<cv::Vec3b>(y, x);
	const QColor c(data[2], data[1], data[0]);
	
	int th = (c.hue() / 2 + 5) % 180;
	int ts = c.saturation() + 40;
	int tv = c.value() + 40;
	int bh = c.hue() / 2 - 5;
	int bs = c.saturation() - 40;
	int bv = c.value() - 40;
	
	if(ts > 255) ts = 255;
	if(tv > 255) tv = 255;
	
	if(bh < 0) bh += 180;
	if(bs < 0) bs = 0;
	if(bv < 0) bv = 0;
	
	qDebug() << "touch bh: " << bh;
	
	m_hsvConfig.setValue("th", th);
	m_hsvConfig.setValue("ts", ts);
	m_hsvConfig.setValue("tv", tv);
	m_hsvConfig.setValue("bh", bh);
	m_hsvConfig.setValue("bs", bs);
	m_hsvConfig.setValue("bv", bv);
	
	refreshHsv();
}

void VisionDialog::currentConfigChanged(const QModelIndex &current, const QModelIndex &prev)
{  
  const bool currentValid = current.isValid();
  const bool prevValid = prev.isValid();
  
  // Save the previously selected config (if exists) in file
  if(prevValid) {
    const std::string path = Camera::ConfigPath::path(m_configModel->fileInfo(prev).baseName().toStdString());
    if(QFile::exists(QString::fromStdString(path)))
      m_channelModel->config().save(path);
  }
  
  // Update interface
  ui->channelsList->selectionModel()->clear();
  ui->removeConfig->setEnabled(currentValid);
  ui->duplicateConfig->setEnabled(currentValid);
  ui->defaultConfig->setEnabled(currentValid && !isDefaultConfig(current));
  ui->addChannel->setEnabled(currentValid);
  ui->addChannel->setEnabled(currentValid);
  ui->channelsList->setEnabled(currentValid);
  
  // Load and set newly selected config
  if(currentValid) {
    Config *config = Config::load(Camera::ConfigPath::path(m_configModel->fileInfo(current).baseName().toStdString()));
    if(config) m_channelModel->setConfig(*config, true);
    delete config;
  }
  else m_channelModel->setConfig(Config(), true);
}

void VisionDialog::currentChannelChanged(const QModelIndex &current, const QModelIndex &prev)
{
  const bool currentValid = current.isValid();
  
  // Update interface
  ui->hsv->setEnabled(currentValid);
  ui->removeChannel->setEnabled(currentValid);
  //ui->up->setEnabled(currentValid && current.row() > 0);
  //ui->down->setEnabled(currentValid && current.row() < m_channelModel->rowCount() - 1);
  
  if(currentValid) {
    Config c = m_channelModel->config();
    c.clearGroup();
    c.beginGroup(CAMERA_GROUP);
    c.beginGroup((CAMERA_CHANNEL_GROUP_PREFIX + QString::number(current.row())).toStdString());
    m_hsvConfig = c.values();
  }
  
  refreshHsv();
}

void VisionDialog::refreshHsv()
{
	blockChildSignals(true);
  
	if(!ui->hsv->isEnabled()) {
		ui->_th->setText(QString());
		ui->_ts->setText(QString());
		ui->_tv->setText(QString());
		ui->_bh->setText(QString());
		ui->_bs->setText(QString());
		ui->_bv->setText(QString());
		return;
	}
	
  // Get the current hsv values
	int th = m_hsvConfig.intValue("th") * 2;
	int ts = m_hsvConfig.intValue("ts");
	int tv = m_hsvConfig.intValue("tv");
	int bh = m_hsvConfig.intValue("bh") * 2;
	int bs = m_hsvConfig.intValue("bs");
	int bv = m_hsvConfig.intValue("bv");

  // Force top and bottom to be different
	if(th == bh) {
		th += 5;
		th %= 360;
		bh -= 5;
		if(bh < 0) bh += 360;
	}
	
	if(ts == bs) {
		ts += 5;
		ts = ts > 255 ? 255 : ts;
		bs -= 5;
		bs = bs < 0 ? 0 : bs;
	}
	
	if(tv == bv) {
		tv += 5;
		tv = tv > 255 ? 255 : tv;
		bv -= 5;
		bv = bv < 0 ? 0 : bv;
	}
	
	// Update the hsv visual widget
	ui->visual->setMax(QColor::fromHsv(th, ts, tv));
	ui->visual->setMin(QColor::fromHsv(bh, bs, bv));
	
	// Update the hsv text fields
	ui->_th->setText(QString::number(th));
	ui->_ts->setText(QString::number(ts));
	ui->_tv->setText(QString::number(tv));
	ui->_bh->setText(QString::number(bh));
	ui->_bs->setText(QString::number(bs));
	ui->_bv->setText(QString::number(bv));
	
	m_device->channels()[0]->setConfig(m_hsvConfig);
  
  // Update the channel within the config
  const QModelIndex current = ui->channelsList->selectionModel()->currentIndex();
  Config c = m_channelModel->config();
  c.clearGroup();
  c.beginGroup(CAMERA_GROUP);
  c.beginGroup(CAMERA_CHANNEL_GROUP_PREFIX + QString::number(current.row()).toStdString());
  c.addValues(m_hsvConfig);
  m_channelModel->setConfig(c);
  
	blockChildSignals(false);
}

/* Keep? */
void VisionDialog::blockChildSignals(const bool block)
{
	ui->_th->blockSignals(block);
	ui->_ts->blockSignals(block);
	ui->_tv->blockSignals(block);
	ui->_bh->blockSignals(block);
	ui->_bs->blockSignals(block);
	ui->_bv->blockSignals(block);
	ui->visual->blockSignals(block);
}
