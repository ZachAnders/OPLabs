#ifndef MERCURY_TESTS_dns_PINGTESTOBSERVER_HPP_
#define MERCURY_TESTS_dns_PINGTESTOBSERVER_HPP_

/*
 * Author: jrahm
 * created: 2015/03/15
 * PingTestObserver.hpp: <description>
 */

#include <mercury/dns/TestResults.hpp>

namespace dns {

/**
 * Interface meant to consume the results of a ping test
 */
class TestObserver {
public:
    /**
     * Called when a test has completed.
     */
    virtual void onTestComplete(const TestResults& results) = 0;
};

}

#endif /* MERCURY_TESTS_dns_PINGTESTOBSERVER_HPP_ */
