from flask import request
from common.auth_model import User
from functools import wraps
from .json_util import JSON_FAILURE
from binascii import unhexlify

# This is the magic cookie the router is expecting
MAGIC_COOKIE_HEX = "e21a14c8a2350a92af1affd6352ba4f39779afb5c12343f0f7141762534aa97e"
MAGIC_COOKIE = unhexlify(MAGIC_COOKIE_HEX)

INVALID_AUTH = (JSON_FAILURE(), 401)

class requires_token():
    """ This is a convenience decorator that will ensure the rapped function will
        only be called if the currently active request contains a valid REST token.
        If it does not, it invokes the 'invalid_handler', which by default
        simply returns an HTTP 401 error to the client. """
    def __init__(self, invalid_handler=None):
        if invalid_handler:
            self.invalid_handler = invalid_handler
        else:
            self.invalid_handler = lambda: INVALID_AUTH

    def __call__(self, route_handler):
        @wraps(route_handler)
        def on_request(*args, **kwargs):
            if 'token' in request.form:
                token = request.form['token']
                this_user = User.get_user(auth_token=token)
                if this_user and this_user.token_matches(token):
                    return route_handler(*args, **kwargs)
            return self.invalid_handler()
        return on_request
