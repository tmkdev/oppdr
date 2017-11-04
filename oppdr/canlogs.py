import logging
import re
import can4python
from can4python.canframe import CanFrame
import pandas as pd

class CanLogHandler(object):
    _logtypes = {'candump': re.compile("\(([0-9]+\.[0-9]+)\) can([0-9]) ([0-9a-fA-F]{3,8})#([0-9a-fA-F]{0,16})"),
                 'cblogger_nobus': re.compile("([0-9]+),([0-9a-fA-F]{2,8}),([0-9a-fA-F,]+)"),
                 'cblogger_bus': re.compile("([0-9]+),([0-9]),([0-9a-fA-F]{1,8}),([0-9a-fA-F,]+)"),
                 }

    def __init__(self, filename, canconfigs):
        self.filename = filename
        self.canconfigs = canconfigs

    def parse(self):
        pdlist = []

        with open(self.filename, 'r') as rawlog:
            for line in rawlog:
                canframe = self.parseframe(line)
                if canframe:
                    if canframe['frameid'] in self.canconfigs[canframe['bus']].framedefinitions:
                        cf = CanFrame(frame_id=canframe['frameid'],
                                      frame_data=canframe['data'],
                                      frame_format=canframe['frameformat'])

                        sigs = cf.unpack(self.canconfigs[canframe['bus']].framedefinitions)
                        sigs['time'] = canframe['timestamp']
                        pdlist.append(sigs)

        return pdlist

    def getdataframe(self):
        pdlist = self.parse()
        pdframe = pd.DataFrame(pdlist)
        pdframe['time'] = pd.to_timedelta(pdframe['time'], unit='S')

        pdframe = pdframe.set_index(['time'])

        return pdframe

    def parseframe(self, frametext):
        packet = None

        for logtype in self._logtypes:
            match = CanLogHandler._logtypes[logtype].match(frametext)
            if match:
                if logtype == 'candump':
                    packet = self.candumpframe(match)

                if logtype == 'cblogger_nobus' or logtype == 'cblogger_bus':
                    packet = self.cbloggerframe(match)

        return packet

    def cbloggerframe(self, frameregex):
        busoffset=0
        bus = 0
        if len(frameregex.groups()) == 4:
            busoffset=1
            bus = int(frameregex.group(2))

        timestamp = float(frameregex.group(1)) / 1000.0
        frameformat = 'extended' if len(frameregex.group(2 + busoffset)) > 4 else 'standard'
        frameid = int(frameregex.group(2 + busoffset), 16)
        stringdata = frameregex.group(3 + busoffset).split(',')
        if stringdata[-1] == '':
            stringdata = stringdata[:-1]

        try:
            bindata = [int(x, 16) for x in stringdata]
        except:
            logging.exception(stringdata)

        packet = {'timestamp': timestamp, 'frameformat': frameformat, 'frameid': frameid, 'data': bindata, 'bus': 0}

        return packet

    def candumpframe(self, frameregex):
        timestamp = float(frameregex.group(1))
        bus = int(frameregex.group(2))
        frameformat = 'extended' if len(frameregex.group(3)) > 4 else 'standard'
        frameid = int(frameregex.group(3), 16)
        payload = frameregex.group(4)
        stringdata = [payload[i:i + 2] for i in range(0, len(payload), 2)]
        bindata = [int(x, 16) for x in stringdata]

        packet = {'timestamp': timestamp, 'frameformat': frameformat, 'frameid': frameid, 'data': bindata, 'bus': bus}

        return packet
