#!/usr/bin/python
# -*- coding: utf-8 -*-
# a simple MySQL class

import mysql.connector
from mysql.connector import errorcode

class JhdbError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class JhdbExcutor:
    ''' a helper class for JhdbPool and Jhdb, do the real MySQL work '''
    def __init__(self, cursor):
        self.cursor = cursor

    def _execute(self, sql):
        try:
            self.cursor.execute(sql)
        except Exception, e:
            raise JhdbError("execute sql error: %s, sql = %s" % (str(e), sql))

    def select(self, sql):
        ''' return a List of Dict : [{},{},...{}] '''
        ret = []
        self._execute(sql)
        cols = self.cursor.column_names
        map(lambda x:ret.append(dict(zip(cols, x))), self.cursor.fetchall())
        return ret

    def update(self, sql):
        ''' return True or False '''
        try:
            self._execute(sql)
        except JhdbError, e:
            print(e)
            return False
        return True

    def insert(self, sql):
        return self.update(sql)

    def delete(self, sql):
        return self.update(sql)

    def delete_by_field(self, table, field, v):
        sql = "DELETE FROM %s WHERE `%s` = '%s'" % (table, field, v)
        return self.delete(sql)

    def update_field(self, table, condition_k, condition_v, field, v):
        sql = "UPDATE %s SET `%s` = '%s' WHERE `%s` = '%s'" % (
                table, field, v, condition_k, condition_v)
        return self.update(sql)

    def get_by_conditions(self, table, conditions, orderby=None, limit=None):
        ''' conditions is a dict of {condition_filed:condition_value} '''
        parsed_conditions = ' AND '.join(
                ["`%s` = '%s'" % (k,v) for k,v in conditions.items()])
        sql = "SELECT * FROM %s WHERE %s" % (table, parsed_conditions)
        if orderby is not None:
            sql = "%s ORDER BY `%s`" % (sql, orderby)
        if limit is not None:
            sql = "%s LIMIT %s" % (sql, limit)
        return self.select(sql)


class JhdbPool:
    ''' Do querys using a limited number of connections '''
    def __init__(self, host, port, user, password, database, pool_size,
            pool_name):
        configs = {
                'host': host,
                'port': port,
                'user': user,
                'password': password,
                'database': database,
                'pool_name': pool_name,
                'pool_size': pool_size,
                'autocommit': True,
                'charset': 'utf8',
                }
        self.cnxpool = mysql.connector.pooling.MySQLConnectionPool(**configs)

    def pool_method(func):
        '''
        We want to do all the work by limited connections,
        so we use a while True try to:
            1. get available connection
            2. get cursor do the SQL query job
            3. return(close) the connection
        This is busy wait, may be not good?
        '''
        def _wrapper(self, *args, **kwds):
            while True:
                try:
                    cnx = self.cnxpool.get_connection()
                    break
                except mysql.connector.errors.PoolError:
                    continue
            excutor = JhdbExcutor(cnx.cursor())
            ret = getattr(excutor, func.__name__)(*args, **kwds)
            cnx.cursor().close()
            cnx.close()
            return ret
        return _wrapper


    @pool_method
    def select(self, sql):
        pass

    @pool_method
    def update(self, sql):
        pass

    @pool_method
    def insert(self, sql):
        pass

    @pool_method
    def delete(self, sql):
        pass

    @pool_method
    def delete_by_field(self, table, field, v):
        pass

    @pool_method
    def update_field(self, table, condition_k, condition_v, field, v):
        pass

    @pool_method
    def get_by_conditions(self, table, conditions, orderby=None, limit=None):
        pass


class Jhdb(JhdbExcutor):
    def __init__(self, host, port, user, password, database):
        configs = {
                'host': host,
                'port': port,
                'user': user,
                'password': password,
                'database': database,
                'autocommit': True,
                'charset': 'utf8',
                }
        try:
            self.cnx = mysql.connector.connect(**configs)
        except mysql.connector.Error as err:
            if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
                raise JhdbError("Access denied")
            elif err.errno == errorcode.ER_BAD_DB_ERROR:
                raise JhdbError("Database %s not exist" % database)
            else:
                raise(err)
        self.cursor = self.cnx.cursor()
        JhdbExcutor.__init__(self, self.cursor)

    def close(self):
        self.cursor.close()
        self.cnx.close()

