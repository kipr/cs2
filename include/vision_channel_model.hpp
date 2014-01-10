#ifndef _VISION_CHANNEL_MODEL_HPP_
#define _VISION_CHANNEL_MODEL_HPP_

#include <QObject>
#include <QStandardItemModel>
#include <kovan/config.hpp>
#include <kovan/camera.hpp>

class VisionChannelModel : public QStandardItemModel
{
Q_OBJECT
public:
	VisionChannelModel(QObject *parent = 0);
	~VisionChannelModel();
	
	Q_PROPERTY(Config config READ config WRITE setConfig)
	void setConfig(const Config &config, const bool completeRefresh = false);
	const Config &config() const;
  
  const QString &channelType(const QModelIndex &index) const;
	
public slots:
	void addChannel(const QString &type);
	void removeChannel(const int i);
	void swapChannels(const int &i, const int &j);
	
	Config channelConfig(const QModelIndex &index) const;
	void setChannelConfig(const QModelIndex &index, const Config &config);
	
private:
	void updateConfig();
	
	Config m_config;
	QString m_blank;
};

#endif
