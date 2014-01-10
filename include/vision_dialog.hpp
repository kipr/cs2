#ifndef _VISION_DIALOG_HPP_
#define _VISION_DIALOG_HPP_

#include <QDialog>
#include <QTimer>
#include <QModelIndex>
#include <QItemSelection>

#include <kovan/config.hpp>

namespace Ui
{
	class VisionDialog;
}

namespace Camera
{
	class Device;
}

class VisionConfigModel;
class VisionChannelModel;

class VisionDialog : public QDialog
{
Q_OBJECT
public:
	VisionDialog(QWidget *parent);
	~VisionDialog();
	
	virtual int exec();
  void setDefault(const QModelIndex &index);
	bool isDefaultConfig(const QModelIndex &index) const;
	
private Q_SLOTS:
	void on_addConfig_clicked();
	void on_removeConfig_clicked();
	void on_duplicateConfig_clicked();
	void on_defaultConfig_clicked();
	void on_addChannel_clicked();
	void on_removeChannel_clicked();
	void on_up_clicked();
	void on_down_clicked();
	
	void updateCamera();
  void visualChanged();
	void manualEntry(const QString &number);
	void imagePressed(const int x, const int y);
	
	void currentConfigChanged(const QModelIndex &current, const QModelIndex &prev);
	void currentChannelChanged(const QModelIndex &current, const QModelIndex &prev);
	
private:
	void refreshHsv();
	void blockChildSignals(const bool block);
	
  Ui::VisionDialog *ui;
	VisionConfigModel *m_configModel;
	VisionChannelModel *m_channelModel;
  
	Camera::Device *m_device;
	Config m_hsvConfig;
	QString m_defaultConfigPath;
	
	QTimer m_cameraTimer;
};

#endif
