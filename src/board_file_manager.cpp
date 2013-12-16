#include "board_file_manager.hpp"
#include <QFileInfo>
#include <QDir>
#include "board_file.hpp"

BoardFileManager::BoardFileManager(QObject *const parent)
  : QObject(parent)
{
}

BoardFileManager::~BoardFileManager()
{
  qDeleteAll(_boards);
}

void BoardFileManager::addBoardFile(BoardFile *const file)
{
  file->setParent(this);
  _boards.push_back(file);
}

bool BoardFileManager::removeBoardFile(BoardFile *const file)
{
  return _boards.removeAll(file);;
}

const QList<BoardFile *> &BoardFileManager::boardFiles() const
{
  return _boards;
}

BoardFile *BoardFileManager::lookupBoardFile(const QString &name)
{
  Q_FOREACH(BoardFile *const board, _boards) {
    if(board->name() == name) return board;
  }
  return 0;
}

void BoardFileManager::addLocation(const QString &path)
{
  _locations.push_back(path);
}

quint32 BoardFileManager::loadLocation(const QString &path)
{
  quint32 added = 0;
  const QList<QFileInfo> files = QDir(path).entryInfoList(QStringList() << "*.board",
    QDir::Files | QDir::NoDot | QDir::NoDotDot);
  Q_FOREACH(const QFileInfo &file, files) {
    BoardFile *const board = BoardFile::load(file.absoluteFilePath());
    if(!board) continue;
    addBoardFile(board);
    ++added;
  }
  return added;
}

void BoardFileManager::reload()
{
  _boards.clear();
  Q_FOREACH(const QString &loc, _locations) loadLocation(loc);
}