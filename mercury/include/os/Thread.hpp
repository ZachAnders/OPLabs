#ifndef THREAD_HPP_
#define THREAD_HPP_

#include <pthread.h>
#include <os/Runnable.hpp>

namespace os {

/** 
 * @brief A simple thread class that accepts a runnable as a delegate.
 *
 * This class should not be extendend.
 * instead such things should be done through the runnable
 * delegate
 *
 * @see Mutex
 * @see ScopedLock
 * @see Condition
 * @see containers::BlockingQueue
 */
class Thread {
public:
    static Thread* getCurrentThread();

    static inline Thread* begin(Runnable& runner) {
        Thread* ret = new Thread(runner);
        ret->start();
        return ret;
    }

    /**
     * @brief Construct a Thread from the given Runnable
     * @param runner The runnable to delegate to
     */
    Thread(Runnable& runner) ;

	/** Start the thread */
	int start();

	/** Join the thread */
	int join();


    /**
     * @brief Return the delegate
     * @return Return the runnable used
     */
	inline Runnable& getRunnable() { return m_runner; }

	inline const Runnable& getRunnable() const { return m_runner; }

    virtual ~Thread();
private:
	Runnable& m_runner;
	pthread_t m_thread;
    bool joined;
};

}

#endif
