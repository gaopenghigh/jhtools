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
    '''linefit for just one List'''
    x = range(1, len(y)+1)
    return linefit(x, y)


def _default_weight_func(offset):
    if offset <= -3 or offset >= 3:
        return 0
    elif offset == 0:
        return 2
    return abs(1/abs(offset))

def smooth(data, weight_func=_default_weight_func):
    '''
    smooth data according to weights geted by function weight_func:
    def weight_func(offset):
        if offset <= -3 or offset >= 3:
            return 0
        elif offset == 0:
            return 2
        return abs(1/abs(offset))
    then, smooth_data[i] = (data[i]*2 + data[i+1]*1 + data[i-1]*1 + \
        data[i-2]*1/2 + data[1+2]*1/2)/(2+1+1+1/2+1/2)
    data is a List of float or int
    return a new List of smoothed data
    '''
    weight_dic = {}
    for off in range(len(data)):
        weight = weight_func(off)
        if weight != 0:
            weight_dic[off] = weight
        weight = weight_func(-off)
        if weight != 0:
            weight_dic[-off] = weight

    ret_data = []
    data_length = len(data)
    for i in range(data_length):
        d = data[i]
        data_sum = 0
        weight_sum = 0
        for off in weight_dic:
            if (i+off) >= 0 and (i+off) < data_length:
                data_sum += data[i+off] * weight_dic[off]
                weight_sum += weight_dic[off]
        if weight_sum == 0:
            ret_data.append(0)
        else:
            ret_data.append(data_sum/weight_sum)

    return ret_data


def test():
    # print linefit_same_step([1,2,3,4])
    data = range(20)
    print smooth(data)


if __name__ == '__main__':
    test()
