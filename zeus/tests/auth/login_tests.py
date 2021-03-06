"""
Authentication tests related to logging in and out.
"""

from util.tests.auth import login, logout, ensure_default_account
from util.tests.output import SUCCESS, FAILURE
from util.tests.context import check_status, Context

def test_valid_login():
    """ Accounts with valid user/pass should be able to login and 
        get an user_token. """
    ensure_default_account()
    resp = login()
    check_status(resp, fields=['user_token'])

    json = resp.json()

    token = json['user_token']

    assert token != None and token != '', FAILURE("Login returned empty user_token!")

    assert len(token) > 64, FAILURE("user_token too small?")

    print (SUCCESS("Valid login test",
                  user=Context.email,
                  passwd=Context.passwd))

    return token

def test_logout(token):
    """ An authenticated token should be able to log itself out, removing it
        from the database. """
    resp = logout(token)
    check_status(resp, fields=['status'])

    assert resp.json()['status'] == 'success', \
        FAILURE("Logout did not return success!")

    print (SUCCESS("Logout test", token=token))

def test_invalid_login(email="fakeemail", pw="INVALIDPASSWORD"):
    """ Invalid credentials should not allow a user to log in. """
    resp = login(email=email, pw=pw)
    check_status(resp, code=401, fields=['status'])

    print (SUCCESS("Invalid login test",
                  user=email,
                  passwd=pw))

