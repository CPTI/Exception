#ifndef MAPUTILS_H
#define MAPUTILS_H

#include <QReadWriteLock>

// funcoes de utilidade que facilita o uso de maps compativeis com a STL


// Overloads para std::map
template<typename K, typename V, typename C, typename A>
bool contains(const std::map<K, V, C, A>& map, const K& key) {
	typename std::map<K, V, C, A>::const_iterator it = map.find(key);
	return (it != map.end());
}


template<typename K, typename V, typename C, typename A>
const V& getValue(const std::map<K, V, C, A>& map, const K& key) {
	typename std::map<K, V, C, A>::const_iterator it = map.find(key);
	if (it != map.end()) {
		return it->second;
	} else {
		throw std::logic_error("No such key");
	}
}


template<typename K, typename V, typename C, typename A>
V& getValue(std::map<K, V, C, A>& map, const K& key) {
	typename std::map<K, V, C, A>::iterator it = map.find(key);
	if (it != map.end()) {
		return it->second;
	} else {
		throw std::logic_error("No such key");
	}
}


template<typename K, typename V, typename C, typename A>
void insert(std::map<K, V, C, A>& map, const K& key, const V& value) {
	map.insert(::std::make_pair(key, value));
}


template<typename K, typename V, typename C, typename A>
bool erase(std::map<K, V, C, A>& map, const K& key) {
	typename std::map<K, V, C, A>::iterator it = map.find(key);
	if (it != map.end()) {
		map.erase(it);
		return true;
	}
	return false;
}

template<typename K, typename V, typename C, typename A, class ListType>
void mapKeys(const std::map<K, V, C, A>& map, ListType& list) {
	typename std::map<K, V, C, A>::const_iterator it = map.begin();
	typename std::map<K, V, C, A>::const_iterator end = map.end();
	for (; it != end; ++it) {
		list.push_back(it->first);
	}
}

template<typename K, typename V, typename C, typename A, class ListType>
void mapValues(const std::map<K, V, C, A>& map, ListType& list) {
	typename std::map<K, V, C, A>::const_iterator it = map.begin();
	typename std::map<K, V, C, A>::const_iterator end = map.end();
	for (; it != end; ++it) {
		list.push_back(it->second);
	}
}

template<typename K, typename V, typename C, typename A, class ListType>
void mapEntrySet(const std::map<K, V, C, A>& map, ListType& list) {
	typename std::map<K, V, C, A>::const_iterator it = map.begin();
	typename std::map<K, V, C, A>::const_iterator end = map.end();
	for (; it != end; ++it) {
		list.push_back(*it);
	}
}


// Overloads para QMap e QHash
template<typename K, typename V, template <class,class> class MapType>
bool contains(const MapType<K,V>& map, const K& key) {
	typename MapType<K,V>::const_iterator it = map.find(key);
	return (it != map.end());
}


template<typename K, typename V, template <class,class> class MapType>
const V& getValue(const MapType<K,V>& map, const K& key) {
	typename MapType<K,V>::const_iterator it = map.find(key);
	if (it != map.end()) {
		return it.value();
	} else {
		throw std::logic_error("No such key");
	}
}


template<typename K, typename V, template <class,class> class MapType>
V& getValue(MapType<K,V>& map, const K& key) {
	typename MapType<K,V>::iterator it = map.find(key);
	if (it != map.end()) {
		return it.value();
	} else {
		throw std::logic_error("No such key");
	}
}


template<typename K, typename V, template <class,class> class MapType>
void insert(MapType<K,V>& map, const K& key, const V& value) {
	map.insert(key,value);
}


template<typename K, typename V, template <class,class> class MapType>
bool erase(MapType<K,V>& map, const K& key) {
	typename MapType<K,V>::iterator it = map.find(key);
	if (it != map.end()) {
		map.erase(it);
		return true;
	}
	return false;
}

template<typename K, typename V, template <class,class> class MapType, class ListType>
void mapKeys(const MapType<K,V>& map, ListType& list) {
	typename MapType<K,V>::const_iterator it = map.begin();
	typename MapType<K,V>::const_iterator end = map.end();
	for (; it != end; ++it) {
		list.push_back(it.key());
	}
}

template<typename K, typename V, template <class,class> class MapType, class ListType>
void mapValues(const MapType<K,V>& map, ListType& list) {
	typename MapType<K,V>::const_iterator it = map.begin();
	typename MapType<K,V>::const_iterator end = map.end();
	for (; it != end; ++it) {
		list.push_back(it.value());
	}
}

template<typename K, typename V, template <class,class> class MapType, class ListType>
void mapEntrySet(const MapType<K,V>& map, ListType& list) {
	typename MapType<K,V>::const_iterator it = map.begin();
	typename MapType<K,V>::const_iterator end = map.end();
	for (; it != end; ++it) {
		list.push_back(::std::make_pair(it.key(), it.value()));
	}
}

// Wrapper thread safe para arrays associativos.
// funciona para QMap, QHash e std::map
template<class MapType>
class ConcurrentMap {
public:
	typedef typename MapType::key_type key_type;
	typedef typename MapType::mapped_type mapped_type;

	bool contains(const key_type& key) const {
		QReadLocker lock(&m_lock);
		return ::contains(m_map, key);
	}

	mapped_type value(const key_type& key) const {
		QReadLocker lock(&m_lock);
		return getValue(m_map, key);
	}

	mapped_type valueOrDefault(const key_type& key) const {
		QReadLocker lock(&m_lock);
		if (::contains(m_map, key)) {
			return getValue(m_map, key);
		}
		return mapped_type();
	}

	void insert(const key_type& key, const mapped_type& value) {
		QWriteLocker lock(&m_lock);
		::insert(m_map, key, value);
	}

	bool erase(const key_type& key) {
		QWriteLocker lock(&m_lock);
		return ::erase(m_map, key);
	}

	QList<key_type> keys() const {
		QReadLocker lock(&m_lock);
		QList<key_type> keys;
		mapKeys(m_map, keys);
		return keys;
	}

	QList<mapped_type> values() const {
		QReadLocker lock(&m_lock);
		QList<mapped_type> values;
		mapValues(m_map, values);
		return values;
	}

	QList<std::pair<key_type, mapped_type> > entrySet() const {
		QReadLocker lock(&m_lock);
		QList<std::pair<key_type, mapped_type> > entries;
		mapEntrySet(m_map, entries);
		return entries;
	}


private:
	MapType m_map;
	mutable QReadWriteLock m_lock;
};


#endif // MAPUTILS_H
