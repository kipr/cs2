#ifndef _BOARD_SELECTOR_DIALOG_HPP_
#define _BOARD_SELECTOR_DIALOG_HPP_

#include <QDialog>
#include <QStandardItemModel>

namespace Ui
{
  class BoardSelectorDialog;
}

class BoardFile;

class BoardSelectorDialog : public QDialog
{
Q_OBJECT
public:
  BoardSelectorDialog(QWidget *const parent = 0);
  ~BoardSelectorDialog();
  
  Q_PROPERTY(QList<BoardFile *> boardFiles READ boardFiles WRITE setBoardFiles)
  void setBoardFiles(const QList<BoardFile *> &boardFiles);
  const QList<BoardFile *> &boardFiles() const;
  
  BoardFile *selectedBoardFile() const;
  
private Q_SLOTS:
  void currentChanged(const QModelIndex &index);
  
private:
  Ui::BoardSelectorDialog *ui;
  QList<BoardFile *> _boards;
  QStandardItemModel *_model;
};

#endif