#!/usr/bin/env python

"""
This module contains all of the models related to user
authentication and administration.

Author: Zach Anders
Date: 01/27/2015
"""

from flask.ext.sqlalchemy import SQLAlchemy
import os
from flask import session, request
import base64
from passlib.apps import custom_app_context as passlib_ctx
from app import db
from models.test_set import TestResult

class User(db.Model):
    """ 
    This model represents a user in the database.
    """
    __tablename__ = "User"
    user_id = db.Column('user_id', db.Integer, primary_key=True)
    email = db.Column('email', db.String(50), unique=True, index=True)
    password = db.Column('password', db.String(256))
    permissions = db.Column('permissions', db.String(50))
    raw_token = db.Column('raw_token', db.String(88))

    @staticmethod
    def get_user(email=None, user_id=None, user_token=None):
        """ Lookup user records by their email address, user_id, or user_token.
            This method does NOT do authentication. """
        if email:
            recs = db.session.query(User).filter(User.email == email).all()
        elif user_id:
            recs = db.session.query(User).filter(User.user_id == user_id).all()
        elif user_token:
            token_id = user_token.split(':')[0]
            recs = db.session.query(User).filter(User.user_id == token_id).all()
        else:
            return None

        if len(recs) != 1:
            return None

        return recs[0]
    
    @staticmethod
    def log_user_in(email, submitted_password):
        """ Attempts to find the user identified by the given email address
            and password. Returns None if this fails for any reason. """
        some_user = User.get_user(email=email)
        if not some_user or not some_user.password_matches(submitted_password):
            return None
        else:
            return some_user

    @staticmethod
    def from_session():
        """ Attempts to load user from the current session. Returns None
            if a valid user could not be found in the current sesssion. """
        if 'email' in session:
            user = User.get_user(email=session['email'])
            if user:
                return user
        return None

    @staticmethod
    def from_user_token():
        if 'user_token' in request.form:
            return User.get_user(user_token=request.form['user_token'])
        return None

    @staticmethod
    def from_router_token():
        ip = request.remote_addr
        token = request.form['router_token'].strip()

        rec = TestResult.get_result_by_token_ip(token, ip)
        
        return rec.parent_set.owner

    def __init__(self, email, password):
        """ Create a user for the given email and password.
            The password is expected to be plaintext, and will be hashed by the
            passlib module. """
        hash_pw = passlib_ctx.encrypt(password)

        self.email = email
        self.password = hash_pw
        self.permissions = ""
        self.raw_token = ""

        db.session.add(self)

    def save(self):
        """ Commits any outstanding changes to the database. """
        db.session.commit()

    def password_matches(self, given_pw):
        """ Checks the given password against the stored password. Returns true
            if they match. """
        return passlib_ctx.verify(given_pw, self.password)

    def token_matches(self, given_token):
        """ Checks the given token against the stored token. Returns true
            if they are valid and match. """
        if self.raw_token:
           return self.get_token() == given_token
        return False

    def delete(self):
        """ Removes this user from the database. """
        db.session.delete(self)
        db.session.commit()

    def new_token(self):
        """ Generate a new token for this user and return it. """
        self.raw_token = base64.urlsafe_b64encode(os.urandom(64))
        db.session.commit()

        return self.get_token()

    def get_token(self):
        """ Fetch this user's whole API token, which is prefixed with their ID. """
        return str(self.user_id) + ":" + self.raw_token
    
    def clear_token(self):
        """ Clear this user's API token, preventing it from being used in the future. """
        self.raw_token = ""
        db.session.commit()
