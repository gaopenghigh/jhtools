#!/usr/bin/python
# -*- coding: utf-8 -*-
# jhhttp, a simple API tool, support RESTful APIs
import simplejson
import urllib
import urllib2
import traceback

class RequestWithMethod(urllib2.Request):
    def __init__(self, url, method, data=None, headers={},\
        origin_req_host=None, unverifiable=False):
        self._method = method
        urllib2.Request.__init__(self, url, data, headers, origin_req_host,
                unverifiable)

    def get_method(self):
        if self._method:
            return self._method
        else:
            return urllib2.Request.get_method(self)


class JhhttpError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)


def _restful_headers(headers=None):
    if headers is None:
        headers = {}
        headers['Content-Type'] = 'application/json'
    else:
        headers['Content-Type'] = 'application/json'
    return headers

def _restful_data(data):
    if type(data) is not str:
        data = simplejson.dumps(data)
    return data

def _prepare_data(data):
    if type(data) is str or data is None:
        return data
    try:
        data = urllib.urlencode(data)
    except Exception, e:
        traceback.print_exc()
        raise JhhttpError(e)
    return data

def _open(url, method, data=None, headers=None):
    if headers is None:
        headers = {}
    try:
        req = RequestWithMethod(url, method, data, headers)
        response = urllib2.urlopen(req)
    except Exception, e:
        traceback.print_exc()
        raise JhhttpError(e)
    return response

def _return(response, response_type):
    try:
        content = response.read()
    except Exception, e:
        traceback.print_exc()
        raise JhhttpError(e)
    if not content:
        return None
    if response_type is 'json':
        try:
            ret = simplejson.loads(content)
        except Exception, e:
            traceback.print_exc()
            raise JhhttpError(e)
    else:
        ret = content
    return ret

def _check(url, data, headers):
    if headers is not None and type(headers) is not dict:
        raise JhhttpError('headers is not a dict')

def _do_http(url, data=None, headers=None, method='GET', response_type='text'):
    try:
        _check(url, data, headers)
        f = _open(url, method, data, headers)
        ret = _return(f, response_type)
    except JhhttpError, e:
        print('JhhttpError, url = %s' % url)
        ret = None
    return ret

def rest_put(url, data=None, headers=None):
    headers = _restful_headers(headers)
    data = _restful_data(data)
    return _do_http(url, data, headers, 'PUT', 'json')

def put(url, data=None, headers=None):
    data = _prepare_data(data)
    return _do_http(url, data, headers, 'PUT', 'text')

def rest_delete(url, data=None, headers=None):
    data = _restful_data(data)
    headers = _restful_headers(headers)
    return _do_http(url, data, headers, 'DELETE', 'json')

def delete(url, data=None, headers=None):
    data = _prepare_data(data)
    return _do_http(url, data, headers, 'DELETE', 'text')

def rest_post(url, data=None, headers=None):
    data = _restful_data(data)
    headers = _restful_headers(headers)
    return _do_http(url, data, headers, 'POST', 'json')

def post(url, data=None, headers=None):
    data = _prepare_data(data)
    return _do_http(url, data, headers, 'POST', 'text')

def rest_get(url, data=None, headers=None):
    data = None
    headers = _restful_headers(headers)
    return _do_http(url, data, headers, 'GET', 'json')

def get(url, data=None, headers=None):
    data = None
    return _do_http(url, data, headers, 'GET', 'text')
