#include "archives_model.hpp"

#include <QStandardItem>
#include <QFileInfo>
#include <QDir>
#include <QFileSystemWatcher>
#include <QDebug>
#include "cs2.hpp"

class ArchiveItem : public QStandardItem
{
public:
	ArchiveItem(const QString &path)
		: m_path(path)
	{
		setText(QFileInfo(path).fileName());
		setIcon(QIcon(":/icons/brick.png"));
    m_id = kiss::Kar::load(m_path)->data(SERVER_ID_FILE);
	}
	
	const QString &path() const
	{
		return m_path;
	}
  
  const QString &id() const
  {
    return m_id;
  }
	
private:
	QString m_path;
  QString m_id;
};

ArchivesModel::ArchivesModel(QObject *parent)
	: QStandardItemModel(parent)
	, m_watcher(new QFileSystemWatcher(this))
{
	connect(m_watcher, SIGNAL(directoryChanged(QString)), SLOT(refresh()));
}

ArchivesModel::~ArchivesModel()
{
}

void ArchivesModel::setArchivesRoot(const QString &archivesRoot)
{
	m_watcher->removePath(m_archivesRoot);
	m_archivesRoot = archivesRoot;
	m_watcher->addPath(m_archivesRoot);
	refresh();
}

const QString &ArchivesModel::archivesRoot() const
{
	return m_archivesRoot;
}

kiss::KarPtr ArchivesModel::archive(const QModelIndex index) const
{
  return kiss::Kar::load(path(index));
}

QString ArchivesModel::path(const QModelIndex index) const
{
	const ArchiveItem *const item = dynamic_cast<const ArchiveItem *>(itemFromIndex(index));
	if(!item) return QString();
	return item->path();
}

QString ArchivesModel::name(const QModelIndex index) const
{
	return QFileInfo(path(index)).fileName();
}

QString ArchivesModel::id(const QModelIndex index) const
{
	const ArchiveItem *const item = dynamic_cast<const ArchiveItem *>(itemFromIndex(index));
	if(!item) return QString();
	return item->id();
}

void ArchivesModel::refresh()
{
	clear();
	const QFileInfo rootInfo(m_archivesRoot);
	if(!rootInfo.isDir()) return;
	
	foreach(const QFileInfo &info, rootInfo.dir().entryInfoList(QDir::Files)) {
		appendRow(new ArchiveItem(info.absoluteFilePath()));
	}
}