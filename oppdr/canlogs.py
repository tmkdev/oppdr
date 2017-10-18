import logging
import re
import can4python
from can4python.canframe import CanFrame
import pandas as pd

class CanLogHandler(object):
    _logtypes = {'candump': re.compile("\(([0-9]+\.[0-9]+)\) can[0-9] ([0-9a-fA-F]{3,8})#([0-9a-fA-F]{0,16})"),
                 'cblogger': re.compile("([0-9]+),([0-9a-fA-F]{1,8}),([0-9a-fA-F,]+)"),
                }

    def __init__(self, filename, canconfig):
        self.filename = filename
        self.canconfig = canconfig

    def parse(self):
        pdlist = []

        with open(self.filename, 'r') as rawlog:
            for line in rawlog:
                canframe = self.parseframe(line)
                if canframe:
                    if canframe['frameid'] in self.canconfig.framedefinitions:
                        cf = CanFrame(frame_id=canframe['frameid'],
                                      frame_data=canframe['data'],
                                      frame_format=canframe['frameformat'])

                        sigs = cf.unpack(self.canconfig.framedefinitions)
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

                if logtype == 'cblogger':
                    packet = self.cbloggerframe(match)

        return packet

    def cbloggerframe(self, frameregex):
        timestamp = float(frameregex.group(1)) / 1000.0
        frameformat = 'extended' if len(frameregex.group(2)) > 4 else 'standard'
        frameid = int(frameregex.group(2), 16)
        stringdata = frameregex.group(3).split(',')
        if stringdata[-1] == '':
            stringdata = stringdata[:-1]

        try:
            bindata = [int(x, 16) for x in stringdata]
        except:
            logging.exception(stringdata)

        packet = {'timestamp': timestamp, 'frameformat': frameformat, 'frameid': frameid, 'data': bindata}

        return packet

    def candumpframe(self, frameregex):
        timestamp = float(frameregex.group(1))
        frameformat = 'extended' if len(frameregex.group(2)) > 4 else 'standard'
        frameid = int(frameregex.group(2), 16)
        payload = frameregex.group(3)
        stringdata = [payload[i:i + 2] for i in range(0, len(payload), 2)]
        bindata = [int(x, 16) for x in stringdata]

        packet = {'timestamp': timestamp, 'frameformat': frameformat, 'frameid': frameid, 'data': bindata}

        return packet
