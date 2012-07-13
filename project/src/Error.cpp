
#include "Error.h"

#include "BackTrace.h"
#include "Exception.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QMutex>
#include <QObject>
#include <QProcess>
#include <QRegExp>
#include <QTemporaryFile>
#include <QTextStream>


#include <iostream>
#include <signal.h>
#include <time.h>

#ifndef __4WIN__
	#include <execinfo.h>
#endif

using namespace ExceptionLib;

//! Nome do arquivo de log.
const QString LOG_FILENAME = "Error.log";

//! Número máximo de entradas na stack trace
const int STACK_MAX_DEPTH = 25;

//! Número de entradas na stack trace para mostrar a causa do warning do qt.
const int QT_WARN_STACK_TRACE = 5;

const QString DATE_FORMAT = "yyyy/MM/dd hh:mm:ss";

//! Descritor do arquivo de log.
QFile& Error::logFile()
{
	static QFile file;
	return file;
}

//! Flag indicando se o log deve ser escrito no console.
bool& Error::logEchoEnabled()
{
	static bool echoEnabled = true;
	return echoEnabled;
}

//! Mutex para garantir atomicidade das operações de log
QMutex* Error::mutex()
{
	static QMutex mutex(QMutex::Recursive);
	return &mutex;
}


const Software* Error::s_mainSoftware = 0;

const Software& Error::mainSoftware()
{
	if (s_mainSoftware == NULL) {
		exit(1);
	}
	return *s_mainSoftware;
}

void Error::initialize(const Software& mainSoftware)
{
	s_mainSoftware = &mainSoftware;

	// Instala o tratador de mensagens do Qt
	qInstallMsgHandler(ERR_qtMessageOutput);

	// Verifica se o diretório do usuário existe, caso contrário cria ele
	QString userDir = QString("%1/%2")
		.arg(QDir::homePath())
		.arg(mainSoftware.name());
	QDir directory;
	if(!directory.exists(userDir)) {
		directory.mkdir(userDir);
	}

	// Abre o arquivo de log
	QString logFileName = QString("%1%2")
		.arg(userDir)
		.arg(LOG_FILENAME);
	logFile().setFileName(logFileName);
	if(!logFile().open(QIODevice::WriteOnly | QIODevice::Append)) {
		// Avisa que não foi possível abrir o arquivo de log
		std::cout << "Nao foi possível abrir o arquivo de log.";
	}

	QString versionData = QString("%1 %2: compilado em %3")
			.arg(mainSoftware.name())
		.arg(mainSoftware.version())
		.arg(mainSoftware.compilationDate());

	Error::log(mainSoftware, "============================================================");
	Error::log(mainSoftware, versionData);
	Error::log(mainSoftware, QString("Revisão: %1").arg(mainSoftware.revision()));
	Error::log(mainSoftware, "============================================================\n");
}


void Error::log(const Software& software, QString message)
{
	QMutexLocker locker(mutex());

	// Monta a mensagem final
	QString finalMessage = QString("%1: %2 - %3\n")
		.arg(software.name())
		.arg(QDateTime::currentDateTime().toString(DATE_FORMAT))
		.arg(message);

	// Escreve no log
	if (logFile().isOpen()) {
		QDataStream stream(&logFile());
		stream << finalMessage;
	}

	// Escreve no console
	if (logEchoEnabled()) {
		std::cout << finalMessage.toStdString();
	}
}


void Error::abortPrivate(
	const Software& software,
	const char * abortFilename,
	unsigned int abortFileLine,
	const QString & additionalInfo)
{
	// Monta o header das informações adicionais
	QString message = QString(
		"ERRO FATAL - O programa será abortado.\n"
		"Versao: %1\n"
		"Revisão do hg: %2\n"
		"Arquivo: %3\n"
		"Linha: %4\n"
		"%5")
		.arg(software.version())
		.arg(software.revision())
		.arg(abortFilename)
		.arg(abortFileLine)
		.arg(additionalInfo);

	QString stacktrace = getStackTrace();
	if(stacktrace != QString::null) {
		message.append("Stacktrace:\n");
		message.append(stacktrace.replace("\n", "\n\t"));
	}

	// Gera o log
	Error::log(software, message);

	printf("%s", message.toLatin1().data());
	//throw AbortException(message);
	exit(1);
}


void Error::abort(
	const Software& software,
	const char * abortFilename,
	unsigned int abortFileLine,
	QString message)
{
	QString additionalInfo = QString(
		"Mensagem: %1\n")
		.arg(message);
	abortPrivate(software, abortFilename, abortFileLine, additionalInfo);
}

void Error::info(
		const Software&, QString message)
{
	QMessageBox::information(
		NULL,
		"INFORMAÇÃO",
		message,
		QMessageBox::Ok);
}


void Error::warn(
		const Software& software,QString message)
{
	Error::log(software, message);
	QMessageBox::warning(
		NULL,
		"AVISO",
		message,
		QMessageBox::Ok);
}



void Error::connect(
		const Software& software,
		const QObject * sender,
		const char * senderAsString,
		const char * signal,
		const QObject * receiver,
		const char * receiverAsString,
		const char * slot,
		Qt::ConnectionType connectionType,
		const char * abortFileName,
		unsigned int abortFileline)
{
	if (!QObject::connect(sender, signal, receiver, slot, connectionType)) {
		QString additionalInfo = QObject::tr(
			"Tipo: Falha em connect\n"
			"sender: %1\n"
			"signal: %2\n"
			"receiver: %3\n"
			"slot: %4\n"
			"connectionType: %5\n")
			.arg(senderAsString)
			.arg(signal + 1)
			.arg(receiverAsString)
			.arg(slot + 1)
			.arg(connectionType);
		abortPrivate(software, abortFileName, abortFileline, additionalInfo);
	}
}


void Error::disconnect(
		const Software& software,
		const QObject * sender,
		const char * senderAsString,
		const char * signal,
		const QObject * receiver,
		const char * receiverAsString,
		const char * slot,
		const char * abortFileName,
		unsigned int abortFileline)
{
	if (!QObject::disconnect(sender, signal, receiver, slot)) {
		QString additionalInfo = QObject::tr(
			"Tipo: Falha em disconnect\n"
			"sender: %1\n"
			"signal: %2\n"
			"receiver: %3\n"
			"slot: %4\n")
			.arg(senderAsString)
			.arg(signal + 1)
			.arg(receiverAsString)
			.arg(slot + 1);
		abortPrivate(software, abortFileName, abortFileline, additionalInfo);
	}
}


#ifdef DEBUG
void Error::assertFailed(
		const Software& software,
		const char * expressionAsString,
		const char * abortFileName,
		unsigned int abortFileline,
		QString message)
{
	QString additionalInfo = QString(
		"Assertiva não satisfeita: %1\n")
		.arg(expressionAsString);

	if(message != QString::null) {
		additionalInfo.append(QString(
			"Mensagem: %1\n")
			.arg(message));
	}

	abortPrivate(
				software,
				abortFileName,
				abortFileline,
				additionalInfo);
}
#endif


void ERR_qtMessageOutput(QtMsgType type, const char * message)
{
	switch (type) {
		case QtDebugMsg:
		case QtWarningMsg:
		case QtCriticalMsg:
		{
			Error::log(Error::mainSoftware(), QString(
				"%1\n"
				"Stacktrace:\n"
				"%2\n")
				.arg(message)
				.arg(getStackTrace()));
			break;
		}
		case QtFatalMsg:
		{
			ERR_ABORT(Error::mainSoftware(), message);
			break;
		}
		default:
		{
			QString detailedMessage = QString(
				"Código de tipo de mensagem desconhecido (%1). Avise\n"
				"o responsável pelo software. Mensagem adicional:\n"
				"%2\n")
				.arg(type)
				.arg(message);
			ERR_ABORT(Error::mainSoftware(), detailedMessage);
			break;
		}
	}
}

QString getStackTrace()
{
	Backtrace::StackTrace* trace = Backtrace::trace();

	QString ret = trace->asString().c_str();
	delete trace;

	return ret;
}
