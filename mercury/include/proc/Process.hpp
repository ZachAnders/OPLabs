#ifndef PROCESS_HPP_
#define PROCESS_HPP_

/*
 * Author: jrahm
 * created: 2015/01/20
 * Process.hpp: <description>
 */

#include <io/FileCollection.hpp>
#include <io/Buffer.hpp>
#include <os/Thread.hpp>
#include <containers/BlockingQueue.hpp>

namespace proc {

/**
 * A process is a type of Thread that has some extra
 * support for asyncronicity.
 *
 * A process is an entity that is global on a per thread
 * basis.
 */
class Process: public os::Runnable {
public:
    Process(const char* name);

    /**
     * return the name of this process
     *
     * @return the name of this process
     */
    inline const char* getName() {
        return name.c_str();
    }

    /**
     * Gets the controlling process of the currently executing
     * thread. May return NULL if the thread has no controlling
     * process
     */
    static Process* getProcess();

    /**
     * Return the main FileCollection used to handle
     * main input and output.
     */
    io::FileCollection& getFileCollection();

    /**
     * @brief Create a new thread under this process
     * @param runner The runnable to run
     * @return A new thread running under this process
     *
     * This should be the preferred method of creating a
     * new process when operating under the process
     * model.
     */
    os::Thread* newThread(Runnable& runner);

    /* the run method. Starts executing a process */
    virtual void run() = 0;


    /**
     * @brief Starts this process in a new thread
     * @return The new thread created by the process
     */
    os::Thread* start();

private:
    io::FileCollection m_file_collection;
    std::string name;
    logger::LogContext& m_log;
};

}

#endif /* PROCESS_HPP_ */
