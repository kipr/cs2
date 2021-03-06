#include "compile_worker.hpp"

#include <kovanserial/kovan_serial.hpp>
#include <pcompiler/pcompiler.hpp>

#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QDebug>

struct Cleaner
{
public:
	Cleaner(const QString& path)
		: path(path)
	{
	}

	~Cleaner()
	{
		remove(path);
	}

private:
	bool remove(const QString& path)
	{
		QDir dir(path);

		if(!dir.exists()) return true;

		QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden
			| QDir::AllDirs | QDir::Files, QDir::DirsFirst);

		foreach(const QFileInfo& entry, entries) {
			const QString entryPath = entry.absoluteFilePath();
			if(!(entry.isDir() ? remove(entryPath) : QFile::remove(entryPath))) return false;
		}

		if(!dir.rmdir(path)) return false;

		return true;
	}

	QString path;
};

CompileWorker::CompileWorker(const kiss::KarPtr &archive, KovanSerial *proto, QObject *parent)
	: QThread(parent),
	m_archive(archive),
	m_proto(proto)
{
}

void CompileWorker::run()
{
	m_output = compile();
	
	qDebug() << "Sending finish!";
	if(!m_proto || !m_proto->sendFileActionProgress(1.0, true)) {
		qWarning() << "send terminal file action progress failed.";
	}
}

const Compiler::OutputList &CompileWorker::output() const
{
	return m_output;
}

void CompileWorker::setUserRoot(const QString &userRoot)
{
	m_userRoot = userRoot;
}

const QString &CompileWorker::userRoot() const
{
	return m_userRoot;
}

void CompileWorker::progress(double fraction)
{
	qDebug() << "Progress..." << fraction;
	if(!m_proto || !m_proto->sendFileActionProgress(false, fraction)) {
		qWarning() << "send file action progress failed.";
	}
}

Compiler::OutputList CompileWorker::compile()
{
	using namespace Compiler;
	using namespace kiss;

	// Extract the archive to a temporary directory
	m_tempDir = tempPath();
	if(!m_archive->extract(m_tempDir)) {
		return OutputList() << Output(m_tempDir, 1,
			QByteArray(), "error: failed to extract KISS Archive");
	}
	
	QStringList extracted;
	foreach(const QString &file, m_archive->files()) extracted << m_tempDir + "/" + file;
	qDebug() << "Extracted" << extracted;

	// Invoke pcompiler on the extracted files
	Engine engine(Compilers::instance()->compilers());
	Options opts = Options::load(QDir::current().filePath("platform.hints"));
	opts.setVariable("${PREFIX}", QDir::current().filePath("prefix"));
	opts.setVariable("${USER_ROOT}", m_userRoot);
	opts.expand();
	
	return engine.compile(Input::fromList(extracted), opts, this);
}

QString CompileWorker::tempPath()
{
	return QDir::tempPath() + "/" + QDateTime::currentDateTime().toString("yyMMddhhmmss") + ".ks2";
}

void CompileWorker::cleanup()
{
	Cleaner c(m_tempDir);
}