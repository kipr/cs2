#include "process_output_buffer.hpp"

#include <QProcess>
#include <QDebug>

ProcessOutputBuffer::ProcessOutputBuffer(QObject *const parent)
  : _process(0)
{
}

void ProcessOutputBuffer::setProcess(QProcess *const process)
{
  _output.clear();
  Q_EMIT outputChanged();
  if(_process) _process->disconnect(this);
  _process = process;
  if(!_process) return;
  connect(_process, &QProcess::readyReadStandardOutput, this, &ProcessOutputBuffer::readyRead);
  connect(_process, &QProcess::readyReadStandardError, this, &ProcessOutputBuffer::readyRead);
}

QProcess *ProcessOutputBuffer::process() const
{
  return _process;
}

QByteArray &ProcessOutputBuffer::output()
{
  return _output;
}

void ProcessOutputBuffer::readyRead()
{
  if(!_process) return;
  _output.append(_process->readAllStandardOutput());
  _output.append(_process->readAllStandardError());
  Q_EMIT outputChanged();
}