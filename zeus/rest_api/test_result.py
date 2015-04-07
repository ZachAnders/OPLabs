"""

This module contains the basic CRUD functionality
for remotely managing test results.

Author: Zach Anders
Date: 04/06/15

"""

from flask import request
from models.test_result import TestResult
from util.json_helpers import JSON_SUCCESS, JSON_FAILURE
from util.rest.rest_auth import requires_user_token
from . import rest_blueprint

@rest_blueprint.route('/test_result/<result_id>/edit')
@requires_user_token()
def edit_result(result_id=None):
    invalid_res = JSON_FAILURE(reason='Invalid Result')
    if not result_id: 
        return invalid_res

    result = TestResult.get_result_by_id(result_id)

    # Columns allowed to be updated and their types
    columns = {'latency_avg':float,
            'latency_sdev':float,
            'jitter_avg':float,
            'jitter_sdev':float,
            'dns_response_avg':float,
            'dns_response_sdev':float,
            'throughput_avg':float,
            'throughput_sdev':float,
            'packet_loss':float,
            'device_name':str,
            'network_type':str,
            'device_ip':str,
            'connection_type':str, }

    for col in columns:
        if col in request.form:
            col_type = columns[col]
            datum = col_type(request.form[col])
            setattr(result, col, datum)
            
    result.save()

    return JSON_SUCCESS()