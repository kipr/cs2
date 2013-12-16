#include "board_selector_dialog.hpp"
#include "ui_board_selector_dialog.h"

#include "board_file.hpp"

#include <QStandardItemModel>
#include <QStandardItem>

class BoardFileItem : public QStandardItem
{
public:
  BoardFileItem(BoardFile *const board);
  
  BoardFile *board() const;
  
  template<typename T>
  static const BoardFileItem *cast(const T *const t);
  
  template<typename T>
  static BoardFileItem *cast(T *const t);
  
private:
  QIcon generateIcon(const quint16 w, const quint16 h) const;
  
  BoardFile *const _board;
};

BoardFileItem::BoardFileItem(BoardFile *const board)
  : _board(board)
{
  setText(_board->name());
  setIcon(generateIcon(64, 64));
}

BoardFile *BoardFileItem::board() const
{
  return _board;
}

template<typename T>
const BoardFileItem *BoardFileItem::cast(const T *const t)
{
  return dynamic_cast<const BoardFileItem *>(t);
}

template<typename T>
BoardFileItem *BoardFileItem::cast(T *const t)
{
  return dynamic_cast<BoardFileItem *>(t);
}

QIcon BoardFileItem::generateIcon(const quint16 w, const quint16 h) const
{
  QPixmap pixmap(w, h);
  QPainter p(&pixmap);
  _board->scene()->render(&p);
  return QIcon(pixmap);
}

BoardSelectorDialog::BoardSelectorDialog(QWidget *const parent)
  : QDialog(parent)
  , ui(new Ui::BoardSelectorDialog)
  , _model(new QStandardItemModel(this))
{
  ui->setupUi(this);
  
  ui->boards->setModel(_model);
  connect(ui->boards->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
    SLOT(currentChanged(QModelIndex)));
}

BoardSelectorDialog::~BoardSelectorDialog()
{
  delete ui;
}

void BoardSelectorDialog::setBoardFiles(const QList<BoardFile *> &boardFiles)
{
  _model->clear();
  _boards = boardFiles;
  Q_FOREACH(BoardFile *const board, _boards) {
    _model->appendRow(new BoardFileItem(board));
  }
  ui->boards->selectionModel()->select(_model->index(0, 0), QItemSelectionModel::Select);
  currentChanged(_model->index(0, 0));
}

const QList<BoardFile *> &BoardSelectorDialog::boardFiles() const
{
  return _boards;
}

BoardFile *BoardSelectorDialog::selectedBoardFile() const
{
  BoardFileItem *const boardItem
    = BoardFileItem::cast(_model->itemFromIndex(ui->boards->currentIndex()));
  if(boardItem) return boardItem->board();
  
  return 0;
}

void BoardSelectorDialog::currentChanged(const QModelIndex &index)
{
  BoardFileItem *const boardItem = BoardFileItem::cast(_model->itemFromIndex(index));
  ui->preview->setScene(0);
  if(!boardItem) return;
  ui->preview->setScene(boardItem->board()->scene());
}