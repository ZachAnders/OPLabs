#ifndef SCOPEDLOCK_HPP_
#define SCOPEDLOCK_HPP_

#include <os/Mutex.hpp>

/*
 * Author: jrahm
 * created: 2014/11/10
 * ScopedLock.hpp: <description>
 */

namespace os {

/**
 * @brief A simple class which locks a mutex for the current scope
 * @author jrahm
 *
 * Example:
 * { ScopedLock __sl(m_mutex)
 *   // critical section
 * }
 */
class ScopedLock {
public:
	inline ScopedLock( Mutex& mutex ) : m_mutex( mutex ) {
		m_mutex.lock();
	}

	inline ~ScopedLock() {
		m_mutex.unlock();
	}

private:
	Mutex& m_mutex;
};

}


#endif /* SCOPEDLOCK_HPP_ */
