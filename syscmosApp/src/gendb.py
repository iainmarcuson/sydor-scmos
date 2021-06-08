import sys


if len(sys.argv) != 3:
    print("Usage: python3 gendb.py <template file> <database file>")
    exit(1)
    
template_filename = sys.argv[1]
query_db_filename = sys.argv[2]

pv_suffix = "_Q"
outpv = "$(P)$(R)OutMux PP";

template_file = open(template_filename, "r");
query_db_file = open(query_db_filename, "w");

for curr_line in template_file:
    base_pv_name = curr_line.split(',')[0].strip();
    new_pv_name = base_pv_name + pv_suffix;
    record_line = 'record(longout, "{}")\n'.format(new_pv_name);
    query_db_file.write(record_line);
    query_db_file.write('{\n');
    field_line = 'field(OUT, "{}")\n'.format(outpv);
    query_db_file.write(field_line);
    if (curr_line.split(',')[1].strip() == "SDLinkStatus"): # ///XXX KLUDGE
        scan_line = 'field(SCAN, "10 second")\n';
        query_db_file.write(scan_line);
    query_db_file.write('}\n\n');


template_file.close();
query_db_file.close();
