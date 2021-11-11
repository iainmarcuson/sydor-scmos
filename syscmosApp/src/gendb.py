import sys


if len(sys.argv) != 2:
    print("Usage: python3 gendb.py <template file>")
    exit(1)
    
template_filename = sys.argv[1]
query_db_filename = "gc_query.db"; # GenCam query database
param_db_filename = "gc_params.db"; # GenCam parameter database

query_suffix = "_Q"

template_file = open(template_filename, "r");
query_db_file = open(query_db_filename, "w");
param_db_file = open(param_db_filename, "w");

for curr_line in template_file:
    if ('#' in curr_line) and not curr_line.startswith('#'):
        print("Comments must take a full line.  # must be the first character in the line.");
        sys.exit(1);

    if '#' in curr_line:
        continue;               # Skip comments

    if curr_line.strip() == '':
        continue;               # Skip blank lines
    
    print("Processing line: {}".format(curr_line.strip()));

    # Extract all the parameters from the line.  Also create derived parameters
    base_pv_name = curr_line.split(',')[0].strip();
    querier_pv_name = base_pv_name + query_suffix;
    asyn_name = curr_line.split(',')[1].strip();
    asyn_query_name = asyn_name + query_suffix;
    pv_type = curr_line.split(',')[2].strip();
    param_var_name = curr_line.split(',')[3].strip();
    param_query_var_name = param_var_name + query_suffix;
    param_var_str_name = curr_line.split(',')[4].strip();
    param_query_str_name = param_var_str_name + query_suffix;
    var_pre_exist = int(curr_line.split(',')[5].strip());
    pv_inhibit = int(curr_line.split(',')[6].strip());
    

    #Generate query PV.  We will always do this
    # The query PV will always be of type longout, since its value is irrelevant
    query_record_line = 'record(longout, "{}")\n'.format(querier_pv_name);
    query_db_file.write(query_record_line);
    query_db_file.write('{\n');
    field_line = 'field(OUT, "@asyn($(PORT),$(ADDR),$(TIMEOUT)){}")\n'.format(asyn_query_name);
    query_db_file.write(field_line);
    field_line = 'field(DTYP, "asynInt32")\n';
    query_db_file.write(field_line);
    field_line = '}\n\n';
    query_db_file.write(field_line);

    if pv_inhibit == 0:         # Generate new parameter PVs
        record_type = ["", ""];
        record_dtyp = "";
        record_suffix = ["", "_RBV"]; # Suffix for setters and getters
        if pv_type == 'd':
            record_type = ["ao", "ai"];
            record_dtyp = "asynFloat64";
        elif pv_type == 'i32':
            record_type = ["longout", "longin"];
            record_dtyp = "asynInt32";
        else:                   # TODO Add string handling
            print("Invalid data type.");
            sys.exit(1);

        #First write the output record
        for record_idx in range(2):
            record_line = 'record({}, "{}")\n'.format(record_type[record_idx], base_pv_name+record_suffix[record_idx]);
            param_db_file.write(record_line);
            param_db_file.write('{\n');
            # TODO Add in support for initial values and PINI
            dtyp_line = 'field(DTYP, "{}")\n'.format(record_dtyp);
            param_db_file.write(dtyp_line);
            if record_idx == 0:
                scan_line = 'field(SCAN, "Passive")\n';
            elif record_idx == 1:
                scan_line = 'field(SCAN, "I/O Intr")\n';
            else:               # Shouldn't be here
                print("Bad state in writing parameter database.");
                sys.exit(1);

            param_db_file.write(scan_line);

            if record_idx == 0:
                addr_line = 'field(OUT, "@asyn($(PORT),$(ADDR),$(TIMEOUT)){}")\n'.format(asyn_name);
            elif record_idx == 1:
                addr_line = 'field(INP, "@asyn($(PORT),$(ADDR),$(TIMEOUT)){}")\n'.format(asyn_name);
            else:               # Shouldn't be here
                print("Bad state in producing address line.\n");
                sys.exit(1);

            param_db_file.write(addr_line);
            out_line = '}\n\n';
            param_db_file.write(out_line);
                
            

template_file.close();
query_db_file.close();

sys.exit(0);

if False:
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
