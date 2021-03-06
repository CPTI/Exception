#ifndef ERROR_H
#define ERROR_H

#ifndef ERRO_H
#define ERRO_H


#include "config.h"
#include "Software.h"
#include "LoggerFwd.h"
#include <stddef.h>
#include <string>

#ifdef SUPPORT_QT
#include <QObject>


class QFile;
class QMutex;
class QString;
#endif

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
	#define ERR_ASSERT(software, exp) do {} while ( false && sizeof(software.name()) < 1 && sizeof(exp) < 1 )
	#define ERR_ASSERT_MESSAGE(software, exp, message) do {} while (false && sizeof(software.name()) < 1 && sizeof(exp) < 1 && sizeof(message) < 1)
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

/* cast<>
 *
 * Um operador para upcast e downcast. Ao contrário do dynamic_cast para ponteiros,
 * para esse cast é um erro se a conversão não funcionar.
 *
 * Em modo debug esse cast usa o dynamic_cast para garantir que a conversão
 * funcionou. Isto é, se o argumento for um ponteiro não-nulo o retorno nunca
 * poderá ser um ponteiro nulo. Caso o conversão não seja possível, levanta
 * uma exceção assim como o dynamic_cast de referências.
 *
 * Em modo release assumimos que o código está suficientemente testado e não contém
 * mais erros de conversão. Assim, neste modo, usamos o static_cast que é muito mais
 * rápido.
 *
 * Nunca use este cast para peguntar se um objeto é de determinado tipo.
 *
 */
#ifdef DEBUG

template<class To, class From>
struct cast_impl;

template<class To, class From>
struct cast_impl<To&, From>
{
	typedef To& ret_type;
	static ret_type cast(const From& f) {
		return dynamic_cast<ret_type>(f);
	}
};


template<class To, class From>
struct cast_impl<const To&, From&>
{
	typedef const To& ret_type;
	static ret_type cast(From& f) {
		return dynamic_cast<ret_type>(f);
	}
};

template<class To, class From>
struct cast_impl<To&, From&>
{
	typedef To& ret_type;
	static ret_type cast(From& f) {
		return dynamic_cast<ret_type>(f);
	}
};


template<class To, class From>
struct cast_impl<To*, From*>
{
	typedef To* ret_type;
	static ret_type cast(From* f) {
		if (f == NULL) { return NULL; }
		// O dynamic_cast de referencias levanta exception
		return &cast_impl<To&, From&>::cast(*f);
	}
};

template<class To, class From>
typename cast_impl<To,From>::ret_type cast(const From& f) {
	return cast_impl<To,From>::cast(f);
}

#else

template<class To, class From>
To cast(const From& f) {
	return static_cast<To>(f);
}

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
    static void initialize(const Software& mainSoftware, std::string outputName = "stderr");

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
            std::string message);
    static void abort(
            const Software& software,
            //! Resultado da macro __FILE__.
            const char *abortFilename,
            //! Resultado da macro __LINE__.
            unsigned int abortFileLine,
            //! Mensagem que será exibida.
            const char* message);
#ifdef SUPPORT_QT
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
#endif

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
            std::string message);
    static void assertFailed(
            const Software& software,
            const char *expressionAsString,
            const char *abortFilename,
            unsigned int abortFileLine,
            const char* message = "");
#ifdef SUPPORT_QT
    static void assertFailed(
            const Software& software,
            const char *expressionAsString,
            const char *abortFilename,
            unsigned int abortFileLine,
            QString message);
#endif
#else
    template<class T>
	static void assertFailed(
			const Software&,
			const char *,
			const char *,
			unsigned int,
            T) {}
#endif

	static const Software& mainSoftware();

	typedef void (*t_abortCallback)(const char *info);
	static void setAbortCallback( t_abortCallback callback );

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
            const std::string &additionalInfo);

    static Log::Logger& errLog(std::string outputName);
    static Log::Logger& errLog(const char* outputName = "stderr");
#ifdef SUPPORT_QT
    static Log::Logger& errLog(QString outputName);
#endif

	static t_abortCallback s_abortCallback;
	static const Software* s_mainSoftware;
};

#endif // ERRO_H


#endif // ERROR_H
