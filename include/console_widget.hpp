#ifndef _CONSOLE_WIDGET_HPP_
#define _CONSOLE_WIDGET_HPP_

#include <QTextEdit>

class QTextStream;
class ProcessOutputBuffer;

class ConsoleWidget : public QTextEdit
{
Q_OBJECT
public:
	ConsoleWidget(QWidget *const parent = 0);
	~ConsoleWidget();
	
	void setProcessOutputBuffer(ProcessOutputBuffer *const processOutputBuffer);
  ProcessOutputBuffer *processOutpuBuffer() const;

signals:
  void abortRequested();

protected:
  void keyPressEvent(QKeyEvent * event);

private Q_SLOTS:
  void read();

private:
  ProcessOutputBuffer *_processOutputBuffer;
  QTextStream *_textStream;
};

#endif
