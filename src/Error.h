#ifndef ERROR_H
#define ERROR_H

#ifndef ERRO_H
#define ERRO_H

#include "Software.h"

#include <QObject>

class QFile;
class QMutex;
class QString;

// Macros
#define ERR_ABORT(software, message) \
	Error::abort(software, __FILE__, __LINE__, message)

#ifdef DEBUG
	#define ERR_ASSERT(software, exp) \
		do { \
			if(!(exp)) { Error::assertFailed(software, #exp, __FILE__, __LINE__); } \
		} while (false)

	#define ERR_ASSERT_MESSAGE(software, exp, message) \
		do { \
			if(!(exp)) { Error::assertFailed(software, #exp, __FILE__, __LINE__, message); } \
		} while (false)
#else // O unico operador que pode ser usado para qualquer coisa é o sizeof.
	#define ERR_ASSERT(software, exp) do {} while ( false && sizeof(exp) < 1 )
	#define ERR_ASSERT_MESSAGE(software, exp, message) do {} while (false && sizeof(exp) < 1 && sizeof(message) < 1)
#endif

#ifdef DEBUG
	#define TRY_CONNECT_MODE(software, sender, signal, receiver, member, mode) \
		Error::connect(software, sender, #sender, signal, receiver, #receiver, member, mode, __FILE__, __LINE__)

	#define TRY_CONNECT(software, sender, signal, receiver, member) \
		Error::connect(software, sender, #sender, signal, receiver, #receiver, member, Qt::AutoConnection, __FILE__, __LINE__)

	#define TRY_DISCONNECT(software, sender, signal, receiver, member) \
		Error::disconnect(software, sender, #sender, signal, receiver, #receiver, member, __FILE__, __LINE__)
#else
	#define TRY_CONNECT_MODE(software, sender, signal, receiver, member, mode) \
        QObject::connect(sender, signal, receiver, member, mode)

	#define TRY_CONNECT(software, sender, signal, receiver, member) \
        QObject::connect(sender, signal, receiver, member, Qt::AutoConnection)

	#define TRY_DISCONNECT(software, sender, signal, receiver, member) \
        QObject::disconnect(sender, signal, receiver, member)
#endif


/*! \brief Singleton que fornece mecanismos para tratar de falhas. */
class Error
{
public:

	/*! \brief Inicializa o sistema de log.
	 *
	 * Este deve ser executado na abertura da aplicação, e é responsável
	 * por abrir o arquivo de log e registrar os sinais de exceção do
	 * sistema.
	 */
	static void initialize(const Software& mainSoftware);

	/*! \brief Gera um evento de log. */
	static void log(
			const Software& software,
			//! Mensagem que será registrada.
			QString message);

	/*! \brief Define o sistema de log deve escrever no console. */
	static void setLogEchoEnabled(
		//! verdadeiro para ativar a escrita.
		bool enabled)
	{
		logEchoEnabled() = enabled;
	}

	/*! \brief Exibe um aviso de erro fatal, e aborta o programa.
	 *
	 * Esta função não deve ser chamada diretamente, a macro ERR_ABORT()
	 * deve ser usada em seu lugar.
	 */
	static void abort(
			const Software& software,
			//! Resultado da macro __FILE__.
			const char *abortFilename,
			//! Resultado da macro __LINE__.
			unsigned int abortFileLine,
			//! Mensagem que será exibida.
			QString message);

	/*! \brief Exibe um popup com a mensagem passada como prâmetro.
	 *
	 * @param	message	Mensagem que será exibida.
	**/
	static void warn(const Software& software, QString message);

	/*! \brief Exibe um popup com a mensagem passada como prâmetro.
	*
	* @param	message	Mensagem que será exibida.
	**/
	static void info(const Software& software, QString message);

	/*! \brief Conecta um sinal a um slot verificando se teve sucesso.
	 *
	 * Esta função não deve ser chamada diretamente, a macro TRY_CONNECT()
	 * deve ser usada em seu lugar.
	 */
	static void connect(
			const Software& software,
			//! Objeto que irá gerar o sinal.
			const QObject *sender,
			//! Representação textual do sender.
			const char *senderAsString,
			//! Sinal a ser conectado.
			const char *signal,
			//! Objeto que irá receber o sinal.
			const QObject *receiver,
			//! Representação textual do receiver.
			const char *receiverAsString,
			//! The slot.
			const char *slot,
			//! Tipo de conexão utilizada.
			Qt::ConnectionType connectionType,
			//! Resultado da macro __FILE__.
			const char *abortFileName,
			//! Resultado da macro __LINE__.
			unsigned int abortFileline);

	/*! \brief Desconecta um sinal a um slot verificando se teve sucesso.
	 *
	 * Esta função não deve ser chamada diretamente, a macro
	 * TRY_DISCONNECT() deve ser usada em seu lugar.
	 */
	static void disconnect(
			const Software& software,
			//! Objeto que irá gerar o sinal.
			const QObject *sender,
			//! Repreentação textual do sender.
			const char *senderAsString,
			//! Sinal a ser conectado.
			const char *signal,
			//! Objeto que irá receber o sinal.
			const QObject *receiver,
			//! Repreentação textual do receiver.
			const char *receiverAsString,
			//! The slot.
			const char *slot,
			//! Resultado da macro __FILE__.
			const char *abortFileName,
			//! Resultado da macro __LINE__.
			unsigned int abortFileline);

	/*! \brief Aborta.
	 *
	 * Esta função não deve ser chamada diretamente, a macro ERR_ASSERT()
	 * deve ser usada em seu lugar.
	 *
	 */
#ifdef DEBUG
	static void assertFailed(
			const Software& software,
			const char *expressionAsString,
			const char *abortFilename,
			unsigned int abortFileLine,
			QString message = QString::null);
#else
	static void assertFailed(
			const Software&,
			const char *,
			const char *,
			unsigned int,
			QString) {}
#endif

	static const Software& mainSoftware();

private:

	/*! \brief Função que realmente aborta o programa levantando uma exceção AbortException.
	 *
	 * \see AbortException
	 */
	static void abortPrivate(
			const Software& software,
			//! Resultado da macro __FILE__.
			const char *abortFilename,
			//! Resultado da macro __LINE__.
			unsigned int abortFileLine,
			//! Informações adicionais da falha.
			const QString &additionalInfo);

	//! Decritor do arquivo de log.
	static QFile& logFile();

	//! Flag indicando se o log deve ser escrito no console.
	static bool& logEchoEnabled();

	//! Mutex para garantir atomicidade das operações de log
	static QMutex* mutex();

	static const Software* s_mainSoftware;
};

/*! \brief Callback do qt utilizada para receber notificações do framework. */
void ERR_qtMessageOutput(
	//! Tipo da mensagem.
	QtMsgType type,
	//! Mensagem.
	const char * message);

/*! \brief Retorna uma string contendo a stacktrace desta chamada
 *
 * \return Stack trace na forma textual ou QString::null em caso de erro.
 */
QString getStackTrace();

#endif // ERRO_H


#endif // ERROR_H
