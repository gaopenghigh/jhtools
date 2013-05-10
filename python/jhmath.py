#!/usr/bin/python
# -*- coding: utf-8 -*-
# gaopenghigh personal math tools

import math
import traceback

def linefit(x, y):
    '''linefit for tow List'''
    # model: y = a * x + b
    # x, y are List
    N = len(x)
    if len(y) != N:
        return
    sx, sy, sxx, syy, sxy = 0, 0, 0, 0, 0
    for i in range(0, N):
        sx  += x[i]
        sy  += y[i]
        sxx += x[i] * x[i]
        syy += y[i] * y[i]
        sxy += x[i] * y[i]
    a = float((sy * sx / N - sxy) / (sx * sx / N - sxx))
    b = float((sy - a * sx) / N)
    try:
        r = float(abs(sy*sx/N-sxy)/math.sqrt((sxx-sx*sx/N)*(syy-sy*sy/N)))
    except Exception, e:
        print 'linefit, get r exception:', e
        r = 0.0
    return a, b, r


def linefit_same_step(y):
    x = range(1, len(y)+1)
    return linefit(x, y)


def test():
    print linefit_same_step([1,2,3,4])


if __name__ == '__main__':
    test()
