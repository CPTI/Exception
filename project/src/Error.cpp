
#include "Error.h"

#include "BackTrace.h"
#include "Exception.h"
#include "Logger.h"

#ifdef SUPPORT_QT
#include <QMessageBox>
#include <QString>
#endif


#include <iostream>
#include <signal.h>
#include <time.h>
#include <sstream>

#ifndef __4WIN__
	#include <execinfo.h>
#endif

using namespace ExceptionLib;

const Software* Error::s_mainSoftware = 0;

const Software& Error::mainSoftware()
{
	if (s_mainSoftware == NULL) {
		std::cout << "Sistema de erros não inicializado. Chame Error::initialize() na função main" << std::endl;
		exit(1);
	}
	return *s_mainSoftware;
}

namespace Log {
	template<>
	struct Formatter<Software> {
        typedef std::string ret_type;
        static ret_type format(const Software& t, const Log::Logger*) {
            return string_format::format("Component: %1, version: %2, revision %3, compilation date: %4",
                    t.name(),
                    t.version(),
                    t.revision(),
                    t.compilationDate());
		}
	};
}
#ifdef SUPPORT_QT
static void ERR_qtMessageOutput(QtMsgType type, const char * message)
{
	static Log::Logger& logger = Log::LoggerFactory::getLogger("Qt log");
	switch (type) {
		case QtDebugMsg:
			logger.log(Log::LDEBUG0, message);
			break;
		case QtWarningMsg:
			logger.log(Log::LWARN, message);
			break;
		case QtCriticalMsg:
			logger.log(Log::LINFO, "%1\n Stacktrace: %2", message, Log::BT);
			break;
		case QtFatalMsg:
			ERR_ABORT(Error::mainSoftware(), message);
			break;
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
#endif
void Error::initialize(const Software& mainSoftware, std::string outputName)
{
	s_mainSoftware = &mainSoftware;
#ifdef SUPPORT_QT
	// Instala o tratador de mensagens do Qt
	qInstallMsgHandler(ERR_qtMessageOutput);
#endif
    errLog(outputName).log(Log::LINFO, "============================================================");
    errLog(outputName).log(Log::LINFO, "%1", mainSoftware);
    errLog(outputName).log(Log::LINFO, "============================================================");
}

Log::Logger& Error::errLog(std::string outputName)
{
    static Log::Logger& log = Log::LoggerFactory::getLogger("Runtime", outputName);
	return log;
}
Log::Logger& Error::errLog(const char* outputName)
{
    return errLog(std::string(outputName));
}
#ifdef SUPPORT_QT
Log::Logger& Error::errLog(QString outputName)
{
    return errLog(outputName.toStdString());
}
#endif
Error::t_abortCallback Error::s_abortCallback = NULL;

void Error::setAbortCallback(t_abortCallback callback)
{
	s_abortCallback = callback;
}

void Error::abortPrivate(
	const Software& software,
	const char * abortFilename,
	unsigned int abortFileLine,
    const std::string & additionalInfo)
{
    errLog().log(Log::LERROR, "FATAL ERROR - The program will be aborted.\n"
				 "%1\n"
                 "File: %2\n"
                 "Line: %3\n"
				 "%4"
				 "Stacktrace:\n%5",
				 software,
				 abortFilename,
				 abortFileLine,
				 additionalInfo,
				 Log::BT);

	if (s_abortCallback){
        (*s_abortCallback)(additionalInfo.c_str());
	}else{
		exit(1);
	}
}


void Error::abort(
	const Software& software,
	const char * abortFilename,
	unsigned int abortFileLine,
    std::string message)
{
    std::stringstream ss;
    ss << "Message: " << message << "\n";
    abortPrivate(software, abortFilename, abortFileLine, ss.str());
}

void Error::abort(
    const Software& software,
    const char * abortFilename,
    unsigned int abortFileLine,
    const char* message)
{
    abort(software, abortFilename, abortFileLine, std::string(message));
}

#ifdef SUPPORT_QT
void Error::abort(
    const Software& software,
    const char * abortFilename,
    unsigned int abortFileLine,
    QString message)
{
    abort(software, abortFilename, abortFileLine, message.toStdString());
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
	errLog().log(Log::LWARN, "%1. %2", software, message);
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
        abortPrivate(software, abortFileName, abortFileline, additionalInfo.toStdString());
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
        abortPrivate(software, abortFileName, abortFileline, additionalInfo.toStdString());
	}
}
#endif


#ifdef DEBUG
void Error::assertFailed(
		const Software& software,
		const char * expressionAsString,
		const char * abortFileName,
		unsigned int abortFileline,
        std::string message)
{
    std::stringstream additionalInfo;
    additionalInfo << "Assertion not satisfied: " << expressionAsString << "\n";


    if(!message.empty()) {
        additionalInfo << "Mensagem: " << message << "\n";
	}

	abortPrivate(
				software,
				abortFileName,
				abortFileline,
                additionalInfo.str());
}


void Error::assertFailed(
        const Software& software,
        const char * expressionAsString,
        const char * abortFileName,
        unsigned int abortFileline,
        const char* message)
{
    assertFailed(software, expressionAsString, abortFileName, abortFileline, std::string(message));
}

#ifdef SUPPORT_QT
void Error::assertFailed(
        const Software& software,
        const char * expressionAsString,
        const char * abortFileName,
        unsigned int abortFileline,
        QString message)
{
    assertFailed(software, expressionAsString, abortFileName, abortFileline, message.toStdString());
}
#endif

#endif



