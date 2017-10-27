import logging

import can4python
import pandas as pd
import yaml
import click

from oppdr.canlogs import CanLogHandler
from oppdr.graphfactory import *


def getpdrconfig(file):
    with open(file, 'r') as stream:
        try:
            config = yaml.load(stream)
        except yaml.YAMLError as exc:
            print(exc)

    return config


def getcanconfigs(kcds):
    canconfigs = []

    for kcd in kcds:
        canconfigs.append(can4python.FilehandlerKcd.read("{0}".format(kcd)))

    return canconfigs

if __name__ == '__main__':
    pdrconfig = getpdrconfig('configs/default_hs.yml')
    canconfigs = getcanconfigs(pdrconfig['kcd'])
    canloghandler = CanLogHandler('logs/00-06-52.csv', canconfigs)

    pdframe = canloghandler.getdataframe()

    pdframe = pdframe.resample('{0}S'.format(pdrconfig['sampletime'])).mean().interpolate(method='linear', limit=10, limit_direction='both')
    pdframe = pdframe.assign(accel=pdframe['speed_average_non_driven_valid'].diff() * 0.277778 / float(pdrconfig['sampletime']))
    pdframe = pdframe.assign(lat_g=pdframe['vehicle_stability_lateral_acceleration']/9.81)
    pdframe = pdframe.assign(long_g=pdframe['accel']/9.81)


    pdframe.to_csv('output/log.csv')

    graphfactory = GraphFactory(pdrconfig, pdframe)
    graphfactory.outputgraphs('output/default_hs.html', showplot=True)
