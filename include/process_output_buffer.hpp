#ifndef _PROCESS_OUTPUT_BUFFER_HPP_
#define _PROCESS_OUTPUT_BUFFER_HPP_

#include <QObject>
#include <QByteArray>

class QProcess;

class ProcessOutputBuffer : public QObject
{
Q_OBJECT
public:
  ProcessOutputBuffer(QObject *const parent = 0);
  
  void setProcess(QProcess *const process);
  QProcess *process() const;
  
  QByteArray &output();
  
private Q_SLOTS:
  void readyRead();
  
Q_SIGNALS:
  void outputChanged();
  
private:
  QProcess *_process;
  QByteArray _output;
};

#endif