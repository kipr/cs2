#ifndef _BOARD_FILE_HPP_
#define _BOARD_FILE_HPP_

#include <QString>
#include <QObject>

class QGraphicsScene;

class BoardFile : public QObject
{
Q_OBJECT
public:
  enum Type
  {
    Real = 0,
    Tape,
    Fake
  };
  
  BoardFile(QObject *const parent = 0);
  
  static BoardFile *load(const QString &path);
  
  Q_PROPERTY(QString name READ name)
  const QString &name() const;
  
  Q_PROPERTY(QGraphicsScene *scene READ scene)
  const QGraphicsScene *scene() const;
  QGraphicsScene *scene();
  
private:
  static void parse(const QString &contents, QGraphicsScene *scene);
  static void error(const quint32 &line, const QString &id);
  static void error(const quint32 &line, const quint16 &expecting, const quint16 &got);
  
  QString _name;
  QGraphicsScene *_scene;
};

#endif
