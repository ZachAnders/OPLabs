"""

This module contains all of the functionality related to interacting
directly with a router.

Author: Zach Anders
Date: 02/21/2015

"""

import os
import socket
from .auth_util import MAGIC_COOKIE, MAGIC_PORT
from IPy import IP

class Router(object):
    """ A utility class for initiating communications with the router
        outside of the standard REST interface. This is necessary in
        case where the Router is idle but we wish to notify it of something.
    """
    def __init__(self, ip_addr):
        self.addr = IP(ip_addr)
        self.req_id = os.urandom(32)

    def wakeup(self):
        """ Attempt to contact the router and wake it up. The expected 
            behavior after this is for the router to attempt to contact
            us back, but by making a REST request to our Web Service.
        """
        if self.addr.version() == 4:
            sock_type = socket.AF_INET
        elif self.addr.version() == 6:
            sock_type = socket.AF_INET6
        else:
            raise ValueError("Unsupported IP address type.")

        host = self.addr.strNormal()
        sock = socket.socket(sock_type, socket.SOCK_STREAM)
        sock.connect((host, MAGIC_PORT))

        sock.sendall(MAGIC_COOKIE + self.req_id)
        
        sock.close()

    def get_id(self):
        """ Returns the generated ID for this router. """
        return self.req_id
