#ifndef _VISION_CONFIG_MODEL_HPP_
#define _VISION_CONFIG_MODEL_HPP_

#include <QFileSystemModel>

class VisionConfigModel : public QFileSystemModel
{
Q_OBJECT
public:
	VisionConfigModel(QObject *parent = 0);
	~VisionConfigModel();
	
	QModelIndex defaultConfiguration() const;
private:
	QFileIconProvider *m_iconProvider;
};

#endif
