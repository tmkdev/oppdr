# oppdr
Open python Performance Data Recorder

Turn vehicle canbus logs into linked HTML based performance charts. 

Output Configuration -> configs/configname.yml
Can Packet Definitions -> kcd.
    Current Definitions understood:
        gm_global_a_hs.kcd: Subset of the HS Global A high speed packets. Aimed at getting vehicle dynamics.
        gm_global_a_ls.kcd: Subset of the HS Global A high speed packets. Aimed at getting GPS and TPMS info.

Dashware:
  data definition for oppdr to dashware import and some gauges I made. Sync video to the canbus data.

Logs:
    I have a couple of sample logs, using a Macchina M2 with the Dual Can logger firmware. Log parser will also
    understand candump formats (great for pi's)
    CANLOG_002 recorded with the Macchina M2 - Dual Can Logger.

M2:
    Collection of arduino sketches for logging Can. Focused on GMLAN Global A collection. Adjust as needed.
    In theory, you could send OBD packets and write a KCD for mapping the responses to data to output/charts.

output:
    Sample chart and csv for Dashware.
    http://htmlpreview.github.io/?https://github.com/tmkdev/oppdr/blob/master/output/CANLOG_002.html

