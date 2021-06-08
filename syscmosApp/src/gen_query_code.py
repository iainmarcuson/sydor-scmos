import sys

if len(sys.argv) != 3:
    print("Usage: python3 gen_query_code.py <template file> <cpp output file>")
    exit(1)

template_filename  = sys.argv[1];
cpp_filename = sys.argv[2];

template_file = open(template_filename, "r");
cpp_file = open(cpp_filename, "w");

#/// TODO Allow specification later, or put in substitution code in IOC
p_str = "dp:";
r_str = "cam1:";

for curr_line in template_file:
    base_pv_name = curr_line.split(',')[0].strip();
    
    out_pv_name = base_pv_name + "_Q";
    #/// Template substitution now handled in IOC, so replacement here unneeded
    #out_pv_name = out_pv_name.replace("$(P)", p_str);
    #out_pv_name = out_pv_name.replace("$(R)", r_str);
    
    param_num_var = curr_line.split(',')[1].strip();
    param_type_name = curr_line.split(',')[2].strip();

    if param_type_name == 'd':
        param_enum = "SD_DOUBLE";
    elif param_type_name == 'i32':
        param_enum = "SD_INT32";
    else:
        param_enum = "SD_STIRNG";
        

    str_line = 'pv_name = "{}";\n'.format(out_pv_name);
    cpp_file.write(str_line);
    replace_p_line = 'template_replace(pv_name, "$(P)", m_prefix_name);\n';
    cpp_file.write(replace_p_line);
    replace_r_line = 'template_replace(pv_name, "$(R)", m_record_name);\n';
    cpp_file.write(replace_r_line);
    printf_line = 'printf("Template string: \\"%s\\"\\n", pv_name.c_str());';
    cpp_file.write(printf_line);
    name_line = 'dbNameToAddr(pv_name.c_str(), &curr_addr);\n'.format(out_pv_name);
    cpp_file.write(name_line);
    param_line = 'param_num = {};\n'.format(param_num_var);
    cpp_file.write(param_line);
    putf_line = 'dbPutField(&curr_addr, DBR_LONG, &param_num, 1);\n';
    cpp_file.write(putf_line);
    printf_line = 'printf("Put num %li for name {}.\\n", param_num);\n'.format(param_num_var);
    cpp_file.write(printf_line);
    dict_line = "pv_dv_types.insert(std::make_pair(param_num, {}));\n".format(param_enum);
    cpp_file.write(dict_line);
    cpp_file.write('\n');
template_file.close();
cpp_file.close();
