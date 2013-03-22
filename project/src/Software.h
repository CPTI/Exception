#ifndef SOFTWARE_H
#define SOFTWARE_H


class Software {
public:
	Software(){}
	virtual ~Software() {}
	virtual const char * name() const = 0;
	virtual const char * version() const = 0;
	virtual const char * revision() const = 0;
	virtual const char * compilationDate() const = 0;
	virtual bool modified() const = 0;
private:
	Software(const Software&);
	Software& operator=(const Software&);
};


class SoftwareImpl: public Software {
public:
	SoftwareImpl(const char * n, const char * v, const char * r, const char * c, bool m)
		: m_name(n), m_version(v), m_revision(r), m_compilationDate(c), m_modified(m) {}
	virtual ~SoftwareImpl() {}
	virtual const char * name() const { return m_name; }
	virtual const char * version() const { return m_version; }
	virtual const char * revision() const { return m_revision; }
	virtual const char * compilationDate() const { return m_compilationDate; }
	virtual bool modified() const { return m_modified; }
private:
	const char * m_name;
	const char * m_version;
	const char * m_revision;
	const char * m_compilationDate;
	bool m_modified;
};

#endif // SOFTWARE_H
