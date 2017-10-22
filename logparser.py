import logging

import can4python
from can4python.canframe import CanFrame
import pandas as pd
import yaml

from oppdr.canlogs import CanLogHandler
from oppdr.graphfactory import *


def getpdrconfig(file):
    with open(file, 'r') as stream:
        try:
            config = yaml.load(stream)
        except yaml.YAMLError as exc:
            print(exc)

    return config


def getcanconfigs(kcd):
    canconfig = can4python.FilehandlerKcd.read("{0}".format(kcd))

    return canconfig

if __name__ == '__main__':
    pdrconfig = getpdrconfig('configs/default_hs.yml')
    canconfig = getcanconfigs(pdrconfig['kcd'])
    canloghandler = CanLogHandler('logs/00-06-52.csv', canconfig)

    pdframe = canloghandler.getdataframe()

    pdframe = pdframe.resample('{0}S'.format(pdrconfig['sampletime'])).mean().interpolate(method='linear', limit=10, limit_direction='both')
    pdframe = pdframe.assign(accel=pdframe['speed_average_non_driven_valid'].diff() * 0.277778 / float(pdrconfig['sampletime']))

    pdframe.to_csv('output/log.csv')

    graphfactory = GraphFactory(pdrconfig, pdframe)
    graphfactory.outputgraphs('output/default_hs.html', showplot=True)
