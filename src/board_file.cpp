#include "board_file.hpp"

#include <QGraphicsScene>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QGraphicsLineItem>

BoardFile::BoardFile(QObject *const parent)
  : QObject(parent)
  , _scene(0)
{
}

BoardFile *BoardFile::load(const QString &path)
{
  BoardFile *boardFile = new BoardFile();
  QFile file(path);
  if(!file.open(QIODevice::ReadOnly)) return 0;
  QGraphicsScene *scene = new QGraphicsScene(boardFile);
  parse(file.readAll(), scene);
  
  boardFile->_scene = scene;
  boardFile->_name = QFileInfo(path).baseName();
  return boardFile;
}

void BoardFile::parse(const QString &contents, QGraphicsScene *scene)
{
#ifndef Q_OS_WIN
	const QStringList lines = contents.split("\n", QString::SkipEmptyParts);
#else
	const QStringList lines = contents.split("\r\n", QString::SkipEmptyParts);
#endif
	quint32 lineNum = 0;
	QPen pen;
	QBrush brush;
	qreal z = 0.0;
	double unitMult = 1.0;
	foreach(const QString &line, lines) {
		++lineNum;
		if(line.startsWith("#")) continue;
		QStringList parts = line.split(" ", QString::SkipEmptyParts);
		quint16 args = parts.size() - 1;
		if(parts[0] == "line") {
			if(args != 4) {
				error(lineNum, 4, args);
				continue;
			}
			QGraphicsItem *item = scene->addLine(parts[1].toDouble() * unitMult, parts[2].toDouble() * unitMult,
				parts[3].toDouble() * unitMult, parts[4].toDouble() * unitMult, pen);
			item->setData(0, BoardFile::Real);
			item->setZValue(z);
		} else if(parts[0] == "dec-line") {
			if(args != 4) {
				error(lineNum, 4, args);
				continue;
			}
			QGraphicsItem *item = scene->addLine(parts[1].toDouble() * unitMult, parts[2].toDouble() * unitMult,
				parts[3].toDouble() * unitMult, parts[4].toDouble() * unitMult, pen);
			item->setData(0, BoardFile::Fake);
			item->setZValue(z);
		} else if(parts[0] == "tape") {
			if(args != 4) {
				error(lineNum, 4, args);
				continue;
			}
			QGraphicsItem *item = scene->addLine(parts[1].toDouble() * unitMult, parts[2].toDouble() * unitMult,
				parts[3].toDouble() * unitMult, parts[4].toDouble() * unitMult, pen);
			item->setData(0, BoardFile::Tape);
			item->setZValue(z);
		} else if(parts[0] == "set-z") {
			if(args != 1) {
				error(lineNum, 1, args);
				continue;
			}
			z = parts[1].toDouble();
		} else if(parts[0] == "dec-rect") {
			if(args != 4) {
				error(lineNum, 4, args);
				continue;
			}
			QGraphicsItem *item = scene->addRect(parts[1].toDouble() * unitMult, parts[2].toDouble() * unitMult,
				parts[3].toDouble() * unitMult, parts[4].toDouble() * unitMult, pen, brush);
			item->setData(0, BoardFile::Fake);
			item->setZValue(z);
		} else if(parts[0] == "pen") {
			if(args != 2) {
				error(lineNum, 2, args);
				continue;
			}
			pen = QPen(QBrush(QColor(parts[1])), parts[2].toUInt());
		} else if(parts[0] == "brush") {
			if(args != 1) {
				error(lineNum, 1, args);
				continue;
			}
			brush = QBrush(QColor(parts[1]));
		} else if(parts[0] == "set-units") {
			if(args != 1) {
				error(lineNum, 1, args);
				continue;
			}
			if(parts[1] == "in") {
				unitMult = 2.54;
			} else if(parts[1] == "cm") {
				unitMult = 1.0;
			} else error(lineNum, parts[1]);
		} else error(lineNum, parts[0]);
	}
}

void BoardFile::error(const quint32 &line, const QString &id)
{
	qWarning() << "Line" << line << "is malformed. Unknown id" << id;
}

void BoardFile::error(const quint32 &line, const quint16 &expecting, const quint16 &got)
{
	qWarning() << "Line" << line << "is malformed. Expected" << expecting << "arguments, got" << got;
}

const QString &BoardFile::name() const
{
  return _name;
}

const QGraphicsScene *BoardFile::scene() const
{
  return _scene;
}

QGraphicsScene *BoardFile::scene()
{
  return _scene;
}