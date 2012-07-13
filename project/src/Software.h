#ifndef SOFTWARE_H
#define SOFTWARE_H


class Software {
public:
	virtual ~Software() {}
	virtual const char * name() const = 0;
	virtual const char * version() const = 0;
	virtual const char * revision() const = 0;
	virtual const char * compilationDate() const = 0;
};

#endif // SOFTWARE_H
