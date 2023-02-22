import sys

NUM_CAPS = 8
pvname_templates = ['"$(P)$(R)Cap{}Gain"', '"$(P)$(R)Cap{}Gain_RBV"']
pvtype_templates = ['mbbo', 'mbbi']
pvaddr_template = '"@asyn($(PORT),$(ADDR),$(TIMEOUT))SD_CAP_GAIN{}"'
pvscan_vals = ['field(SCAN, "Passive")', 'field(SCAN, "I/O Intr")']
addr_templates = ['field(OUT, {})', 'field(INP, {})']
out_filename = 'gc_keck_gains.db';
out_file = open(out_filename, 'w');

enum_str = '''
field(ZRVL, "2")
field(ZRST, "1.000")
field(ONVL, "8")
field(ONST, "0.644")
field(TWVL, "4")
field(TWST, "0.600")
field(THVL, "1")
field(THST, "0.429")
field(FRVL, "10")
field(FRST, "0.392")
field(FVVL, "6")
field(FVST, "0.375")
field(SXVL, "12")
field(SXST, "0.311")
field(SVVL, "3")
field(SVST, "0.300")
field(EIVL, "9")
field(EIST, "0.257")
field(NIVL, "5")
field(NIST, "0.250")
field(TEVL, "14")
field(TEST, "0.237")
field(ELVL, "11")
field(ELST, "0.205")
field(TVVL, "7")
field(TVST, "0.200")
field(TTVL, "13")
field(TTST, "0.180")
field(FTVL, "15")
field(FTST, "0.153")
'''

for cap_idx in range(NUM_CAPS):
    cap_num = cap_idx+1;        # Cap PVs are one-indexed

    for dir_idx in range(2):    # One out, one in
        record_name = pvname_templates[dir_idx].format(cap_num);
        record_string = 'record({}, {})\n'.format(pvtype_templates[dir_idx], record_name)
        out_file.write(record_string);
        out_file.write('{\n')
        out_file.write('field(DTYP, "asynInt32")\n') # Enums are int type
        out_file.write(pvscan_vals[dir_idx])
        out_file.write('\n')
        pv_addr = pvaddr_template.format(cap_num)
        addr_string = addr_templates[dir_idx].format(pv_addr)
        out_file.write(addr_string)
        out_file.write('\n')
        out_file.write(enum_str)
        out_file.write('}\n\n')
        
    
out_file.close()
