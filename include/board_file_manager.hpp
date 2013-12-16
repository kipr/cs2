#ifndef _BOARD_FILE_MANAGER_HPP_
#define _BOARD_FILE_MANAGER_HPP_

#include <QObject>
#include <QList>
#include <QStringList>

class BoardFile;

class BoardFileManager : public QObject
{
public:
  BoardFileManager(QObject *const parent = 0);
  ~BoardFileManager();
  
  void addBoardFile(BoardFile *const file);
  bool removeBoardFile(BoardFile *const file);
  
  const QList<BoardFile *> &boardFiles() const;
  
  BoardFile *lookupBoardFile(const QString &name);
  
  void addLocation(const QString &path);
  quint32 loadLocation(const QString &path);
  
  void reload();
private:
  QList<BoardFile *> _boards;
  QStringList _locations;
};

#endif