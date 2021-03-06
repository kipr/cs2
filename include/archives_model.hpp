#ifndef _ARCHIVES_MODEL_HPP_
#define _ARCHIVES_MODEL_HPP_

#include <QStandardItemModel>
#include <QString>

#include <kar/kar.hpp>

class QFileSystemWatcher;

class ArchivesModel : public QStandardItemModel
{
Q_OBJECT
public:
	ArchivesModel(QObject *parent);
	~ArchivesModel();
	
	Q_PROPERTY(QString archivesRoot READ archivesRoot WRITE setArchivesRoot);
	void setArchivesRoot(const QString &archivesRoot);
	const QString &archivesRoot() const;
	
  kiss::KarPtr archive(const QModelIndex index) const;
	QString path(const QModelIndex index) const;
	QString name(const QModelIndex index) const;
  QString id(const QModelIndex index) const;
  const QModelIndex indexFromName(const QString &name) const;
	
private Q_SLOTS:
	void refresh();
	
private:
	QString m_archivesRoot;
	QFileSystemWatcher *m_watcher;
};

#endif
