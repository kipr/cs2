#include "console_widget.hpp"
#include "process_output_buffer.hpp"

#include <QApplication>
#include <QKeyEvent>
#include <QTextStream>
#include <QProcess>
#include <QDebug>

ConsoleWidget::ConsoleWidget(QWidget *const parent)
  : QTextEdit(parent)
  , _processOutputBuffer(0)
  , _textStream(0)
{
  setProcessOutputBuffer(0);
}

ConsoleWidget::~ConsoleWidget()
{
  setProcessOutputBuffer(0);
}

void ConsoleWidget::setProcessOutputBuffer(ProcessOutputBuffer *const processOutputBuffer)
{
  clear();
  delete _textStream;
  _textStream = 0;
  if(_processOutputBuffer) _processOutputBuffer->disconnect(this);
  _processOutputBuffer = processOutputBuffer;
  if(!_processOutputBuffer) return;
  _textStream = new QTextStream(&_processOutputBuffer->output());
  connect(_processOutputBuffer, SIGNAL(outputChanged()), this, SLOT(read()));
}

ProcessOutputBuffer *ConsoleWidget::processOutpuBuffer() const
{
  return _processOutputBuffer;
}

void ConsoleWidget::keyPressEvent(QKeyEvent *event)
{
  if(!_processOutputBuffer) return;
  QTextEdit::keyPressEvent(event);
  if(event->modifiers() != Qt::NoModifier && event->modifiers() != Qt::ShiftModifier) return;
  QString text = event->text();
  // if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) text = "\n";
  
  _processOutputBuffer->process()->write(text.toUtf8(), text.size());
}

void ConsoleWidget::read()
{
  if(_processOutputBuffer->output().isEmpty()) clear();
  insertPlainText(_textStream->readAll());
  moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);
}
