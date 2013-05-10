#!/usr/bin/python
# -*- coding: utf-8 -*-
# a simple MySQL class

import MySQLdb
import traceback

class Jhdb:
    def __init__(self, host, port, user, passwd, db):
        try:
            conn = MySQLdb.connect(host=host, port=int(port), user=user, \
                                      passwd=passwd, db=db)
            conn.autocommit(True)
            conn.set_character_set('utf8')
            self.cursor = conn.cursor()
        except Exception, e:
            print "Jhdb connectiong error, %s" % str(e)
            traceback.print_exc()


    def select(self, sql):
        '''
        return a List of Dict : [{},{},...{}]
        '''
        ret = []
        try:
            self.cursor.execute(sql)
        except Exception, e:
            print "Jhdb error %s, sql = %s" % (str(e), sql)
            return ret
        cols = [desc[0] for desc in self.cursor.description]
        for d in self.cursor.fetchall():
            dic = {}.fromkeys(cols)
            for (name, value) in zip(cols, d):
                dic[name] = value
            ret.append(dic)
        return ret

    def update(self, sql):
        ''' return True or False '''
        try:
            self.cursor.execute(sql)
            return True
        except Exception, e:
            print "Jhdb error %s, sql = %s" % (str(e), sql)
            return False

    def insert(self, sql):
        return self.update(sql)

    def delete(self, sql):
        return self.update(sql)


def test():
    db = Jhdb(host="10.20.174.102", \
              port="3306", \
              user="root", \
              passwd="123", \
              db="aliddin"
              )
    print db.select("SELECT * FROM uedpub")


if __name__ == "__main__":
    test()

