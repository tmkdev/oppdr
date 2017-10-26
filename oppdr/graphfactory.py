import logging

from bokeh.plotting import *
from bokeh.models import HoverTool


def getgraphparm(config, graph, key):
    if key in config['graphs'][graph]:
        return config['graphs'][graph][key]
    else:
        return config['graph_default'].get(key, None)


class GraphFactory(object):
    def __init__(self, pdrconifg, pddata):
        self.pddata = pddata
        self.pdrconfig = pdrconifg
        self.timehover = HoverTool(tooltips=[
                ("Time", "$x"),
                ("Value", "$y"),
            ])
        self.linhover = HoverTool(tooltips=[
                ("Lat(g)", "$x"),
                ("Long(g)", "$y"),
            ])



    def outputgraphs(self, outputfile, showplot=True):
        output_file(outputfile, title="OPPDR Plots")
        glist = []

        ds = ColumnDataSource(self.pddata)

        for graph in self.pdrconfig['graphs']:
            graphobj = self.pdrconfig['graphs'][graph]

            hover = self.timehover
            x_axis_type = getgraphparm(self.pdrconfig, graph, 'x_axis_type')
            if x_axis_type == 'linear':
                hover = self.linhover

            g = figure(tools=self.pdrconfig['graph_default']['tools'],
                       title=graphobj['title'],
                       x_axis_label=getgraphparm(self.pdrconfig, graph, 'x_axis_label'),
                       x_axis_type=x_axis_type,
                       y_axis_label=graphobj['y_axis_label'],
                       output_backend="webgl")

            if 'x_range' in graphobj:
                g.x_range = glist[int(graphobj['x_range'])].x_range

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

        plotobj = gridplot(glist, ncols=3)

        if showplot:
            show(plotobj)
        else:
            save(plotobj)


