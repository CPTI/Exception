#ifndef NOTNULL_H
#define NOTNULL_H

#include "config.h"
#include "Error.h"

template<typename T>
class NotNull {

	// Tipo propositalmente incompleto, ver construtor NotNull(int) abaixo
	struct NullPointerLiteralNotAllowed;

public:
	// Definicoes de tipo que podem ser úteis em outros templates
	typedef T              value_t;
	typedef const T        const_value_t;
	typedef value_t*       ptr_t;
	typedef const_value_t* const_ptr_t;
	typedef value_t&       ref_t;
	typedef const_value_t& const_ref_t;

	// O compilador só tenta criar código para este construtor quando alguem usa NotNull(0) ou NotNull(NULL)
	// e neste caso o programa não compila. Ninguém em sã consciência tentaria fazer isso deste jeito, mas
	// usando conversão implícita ( foo(NULL) ), isso pode acontecer e é bom a gente impedir em tempo de compilação.
	NotNull(int) { NullPointerLiteralNotAllowed(); }

	/**
	  * @brief	Construtor, que verifica na construção se o ponteiro passado é NULL
	 **/
	NotNull(ptr_t ptr) : m_ptr(ptr) {
		ERR_ASSERT_MESSAGE(Error::mainSoftware(), m_ptr != NULL, "Violated Not-Null contract");
	}

	/**
	  * @brief	Construtor de cópia. A princípio a verificação do ponteiro do objeto a ser copiado
	  *			é redundante, mas se descobrirmos que o ponteiro é NULL, então algo muito estranho
	  *			aconteceu que vale a pena ser detectada
	 **/
	NotNull(const NotNull& that) : m_ptr(that.m_ptr) {
		ERR_ASSERT_MESSAGE(Error::mainSoftware(), m_ptr != NULL, "Violated Not-Null contract, possible memory corruption");
	}

	/**
	  * @brief	Operador de atribuição. Como nosso objeto funciona como um ponteiro seguro, faz sentido permitir
	  *			a atribuição. Note que a verificação acontece no construtor de cópia.
	 **/
	NotNull& operator=(NotNull that) {
		std::swap(*this, that);
		return *this;
	}

	//! @brief	Destrutor
	~NotNull() {}

	//! @brief	Operador *. Simplesmente retorna o ponteiro
	ref_t operator*() const { return *m_ptr; }

	//! @brief	Operador *. Simplesmente delega para o objeto apontado
	ptr_t operator->() const { return m_ptr; }

	//! @brief	Operador de conversão implícita para ponteiro
	operator ptr_t() const { return m_ptr; }

	//! @brief	Operador de conversão implícita para NotNull const
	operator NotNull<const_value_t>() const { return NotNull<const_value_t>(m_ptr); }

	//! @brief	Método para retornar o ponteiro em casos em que a conversão implícita não é aplicável (e.g. dynamic_cast)
	ptr_t ptr() const { return m_ptr; }

private:
	// ponteiro
	ptr_t m_ptr;
	friend void std::swap<>(NotNull<T>&, NotNull<T>&);
};

#ifdef SUPPORT_QT
//! Overload da funcao qHash que repassa para o qHash de ponteiro
template <class T> inline uint qHash(const NotNull<T>& n)
{
	return qHash(n.ptr());
}
#endif

namespace std {
	/**
	  * @brief Overload da função std::swap para NotNull
	 **/
	template <typename T>
	void swap(NotNull<T>& nn1, NotNull<T>& nn2) {
		std::swap(nn1.m_ptr, nn2.m_ptr);
	}
}

#endif // NOTNULL_H
