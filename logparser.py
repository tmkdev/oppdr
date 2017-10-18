import logging
import re

import can4python
from can4python.canframe import CanFrame

from bokeh.plotting import *
from bokeh.models import HoverTool
import pandas as pd
import yaml

from opddr.oppdr.canlogs import CanLogHandler

def getgraphparm(config, graph, key):
    if key in config['graphs'][graph]:
        return config['graphs'][graph][key]
    else:
        return config['graph_default'].get(key, None)


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

def resamplepdframe(frame, interval):
    frame = frame.resample('{0}S'.format(interval)).mean().interpolate(method='linear', limit=10, limit_direction='both')
    frame = frame.assign(accel=pdframe['speed_average_non_driven_valid'].diff() * 0.277778 / interval)

    return frame

if __name__ == '__main__':
    pdrconfig = getpdrconfig('configs/default_hs.yml')
    canconfig = getcanconfigs(pdrconfig['kcd'])
    canloghandler = CanLogHandler('logs/00-06-52.csv', canconfig)

    pdframe = canloghandler.getdataframe()

    #pdframe = resamplepdframe(pdframe, int(pdrconfig['sampletime']))
    pdframe = pdframe.resample('{0}S'.format(pdrconfig['sampletime'])).mean().interpolate(method='linear', limit=10, limit_direction='both')
    pdframe = pdframe.assign(accel=pdframe['speed_average_non_driven_valid'].diff() * 0.277778 / float(pdrconfig['sampletime']))

    ds = ColumnDataSource(pdframe)

    output_file('output/default_hs.html')

    glist = []

    for graph in pdrconfig['graphs']:
        graphobj = pdrconfig['graphs'][graph]

        hover = HoverTool(tooltips=[
            ("Time", "$x"),
            ("Value", "$y"),
            ],
        )

        g = figure(tools=pdrconfig['graph_default']['tools'],
                       title=graph,
                       x_axis_label=getgraphparm(pdrconfig, graph, 'x_axis_label'),
                       x_axis_type=getgraphparm(pdrconfig, graph, 'x_axis_type'),
                       y_axis_label=graphobj['y_axis_label'],
                       output_backend="webgl")

        g.add_tools(hover)

        for series in graphobj['series']:
            sdict = graphobj['series'][series]

            if 'line' in sdict['type']:
                g.line(x=sdict['x'], y=sdict['y'], color=sdict['color'], source=ds, legend=series)
            if 'circle' in sdict['type']:
                g.circle(x=sdict['x'], y=sdict['y'], color=sdict['color'], source=ds, size=2, legend=series)
            if 'cross' in sdict['type']:
                g.cross(x=sdict['x'], y=sdict['y'], color=sdict['color'], source=ds, size=2, legend=series)

        g.legend.location = "top_left"
        g.legend.click_policy = "hide"

        glist.append(g)

    show(gridplot(glist, ncols=3))