import logging
import os
import math

import can4python
import pandas as pd
import yaml
import click

from oppdr.canlogs import CanLogHandler
from oppdr.graphfactory import *

import click

@click.group(chain=True)
@click.option('--debug/--no-debug', default=False)
@click.option('--config', default='configs/default.yml')
@click.option('--kcd_path', default='kcd')
@click.option('--logpath', default='logs',)
@click.option('--outputpath', default='output')
@click.option('--log',  required=True)
@click.option('--showgraph', default=True)
@click.pass_context
def cli(ctx, debug, config, kcd_path, logpath, outputpath, log, showgraph):
    """
    Testing doc string
    """
    ctx.obj['DEBUG'] = debug
    ctx.obj['CONFIG'] = config
    ctx.obj['KCD_PATH'] = kcd_path
    ctx.obj['LOGPATH'] = logpath
    ctx.obj['OUTPUTPATH'] = outputpath
    ctx.obj['LOG'] = log
    ctx.obj['SHOWGRAPH'] = showgraph

    pdrconfig = getpdrconfig(config)
    canconfigs = getcanconfigs(pdrconfig['kcd'])
    canloghandler = CanLogHandler('{0}/{1}'.format(logpath, log), canconfigs)
    ctx.obj['PDRCONFIG'] = pdrconfig

    click.echo('Starting log processing')
    pdframe = canloghandler.getdataframe()

    pdframe = pdframe.ffill().resample('{0}S'.format(pdrconfig['sampletime'])).mean()

    pdframe = pdframe.assign(accel=pdframe['speed_average_non_driven_valid'].diff() * 0.277778 / float(pdrconfig['sampletime']))
    pdframe = pdframe.assign(lat_g=pdframe['vehicle_stability_lateral_acceleration']/9.81)
    pdframe = pdframe.assign(long_g=pdframe['accel']/9.81)


    click.echo('Finished log processing')

    ctx.obj['PD_FRAME'] = pdframe



@cli.command('dumplog')
@click.pass_context
def dumplog(ctx):
    click.echo('Exporting dumplog')
    baselog = os.path.splitext(ctx.obj['LOG'])[0]

    ctx.obj['PD_FRAME'].to_csv('{0}/{1}.csv'.format(ctx.obj['OUTPUTPATH'], baselog))


@cli.command('graph')
@click.pass_context
def graph(ctx):
    click.echo('Generating graphs')
    baselog = os.path.splitext(ctx.obj['LOG'])[0]

    graphfactory = GraphFactory(ctx.obj['PDRCONFIG'], ctx.obj['PD_FRAME'])
    graphfactory.outputgraphs('{0}/{1}.html'.format(ctx.obj['OUTPUTPATH'], baselog),
                              showplot=ctx.obj['SHOWGRAPH'])

def getpdrconfig(file):
    config = None
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
    cli(obj={})