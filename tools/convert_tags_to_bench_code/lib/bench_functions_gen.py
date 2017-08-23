import sys
import os
import re
from __builtin__ import len

PARSED_OPTS_VAR = "reprompib_opts"
JOB_VAR_NAME = "reprompib_job"
NREP_INDEX_VAR_NAME = "reprompib_nrep_index"
SYNCF_VAR_NAME = "reprompib_sync_f"

def generate_init_bench(indent):
    code = [ "reprompib_initialize_benchmark(argc, argv, &%s, &%s);" % (SYNCF_VAR_NAME, PARSED_OPTS_VAR)
             ]
    return format_code(code, indent)


def generate_init_sync(indent):
    code = [ "%s.sync_clocks();" % (SYNCF_VAR_NAME),
            "%s.init_sync();" % (SYNCF_VAR_NAME)
            ]
    return format_code(code, indent)



def generate_start_sync(indent):
    code = [ "%s.start_sync();" % (SYNCF_VAR_NAME) ]
    return format_code(code, indent)


def generate_stop_sync(indent):
    code = [ "%s.stop_sync();" % (SYNCF_VAR_NAME) ]
    return format_code(code, indent)


def generate_measure_timestamp(ts, indent):
    code = [ "%s[%s] = %s.get_time();" % (ts, NREP_INDEX_VAR_NAME, SYNCF_VAR_NAME) ]
    return format_code(code, indent)



def generate_print_output(output_config, indent):
    code = [ "reprompib_print_bench_output(%s, %s, %s, "   % (JOB_VAR_NAME, output_config["start_time"], output_config["end_time"]),
             "        %s, %s,"                   % (SYNCF_VAR_NAME, PARSED_OPTS_VAR),
             "        \"%s\", \"%s\", \"%s\");"  % (output_config["op"], output_config["name"], output_config["type"])
            ]

    return generate_init_job(output_config["string_list"], output_config["int_list"], indent) + \
            format_code(code, indent) + generate_cleanup_job(indent) + "\n"


def generate_cleanup_sync(indent):
    code = [ "%s.clean_sync_module();" % (SYNCF_VAR_NAME) ]
    return format_code(code, indent)


def generate_cleanup_variables(ts_array, strings_array, indent):
    cleanup_ts_arrays_code = generate_cleanup_arrays(ts_array, indent)
    cleanup_strings_array_code = generate_cleanup_arrays(strings_array, indent)

    return cleanup_ts_arrays_code + cleanup_strings_array_code + "\n"



def generate_cleanup_bench(indent):
    cleanup_sync_code = generate_cleanup_sync(indent)

    code = [ "reprompib_cleanup_benchmark(%s);" % (PARSED_OPTS_VAR) ]

    return cleanup_sync_code + format_code(code, indent)



def generate_declare_variables_main_file(indent):

    code = [ "reprompib_sync_functions_t %s;" % (SYNCF_VAR_NAME),
#             "int %s = 1;" % (NREP_VAR_NAME),
#             "int %s = 0;" % (NREP_INDEX_VAR_NAME),
             "reprompib_options_t %s;" % (PARSED_OPTS_VAR)
            ]
    return format_code(code, indent)



def generate_declare_variables(main_file, ts_arrays, strings_array, indent):

    if main_file:
        return generate_declare_variables_main_file(indent)

    ts_array_unique = list(set(ts_arrays))
    additional_vars1 = map((lambda t: "double* %s = NULL;" % (t) ), ts_array_unique)
    strings_array_unique = list(set(strings_array))
    additional_vars2 = map((lambda s: "char* %s = NULL;" % (s) ), strings_array_unique)

    code = [ "extern reprompib_sync_functions_t %s;" % (SYNCF_VAR_NAME),
             "extern reprompib_options_t %s;" % (PARSED_OPTS_VAR),
             "reprompib_job_t %s;" % (JOB_VAR_NAME),
             "int %s;" % (NREP_INDEX_VAR_NAME)
             ]
    code.extend(additional_vars1)
    code.extend(additional_vars2)

    return format_code(code, indent)



def generate_add_includes(indent):
    code = [ "#include <string.h>",
            "#include \"reprompi_bench/sync/synchronization.h\"",
            "#include \"reprompi_bench/benchmark_lib/reproMPIbenchmark.h\"",
            "#include \"reprompi_bench/utils/keyvalue_store.h\""
            ]
    return format_code(code, indent)



def generate_init_timestamp_array(name, indent):
    code = [ "%s = (double*) calloc(%s.n_rep, sizeof(double));" % (name, PARSED_OPTS_VAR) ]
    return format_code(code, indent)



def generate_cleanup_arrays(array, indent):
    array_unique = list(set(array))
    code_free = map((lambda t: "free(%s);" % (t) ), array_unique)
    code_set_null = map((lambda t: "%s = NULL;" % (t) ), array_unique)
    return format_code(code_free, indent) + format_code(code_set_null, indent)



def generate_init_job(svars, ivars, indent):

    print "\n\n$$$$$$$$$$$$$$$$$$$$$$$$"
    print "svars:", svars
    print "ivars:", ivars
    print "#########################"

    job_config = [
            "reprompib_initialize_job(%s.n_rep, &%s);" % (PARSED_OPTS_VAR, JOB_VAR_NAME)
            ]
    job_config.extend( map((lambda (key, val): "reprompib_add_svar_to_job(\"%s\", %s, &%s);" % (key, val, JOB_VAR_NAME) ), svars.iteritems()))
    job_config.extend( map((lambda  (key, val): "reprompib_add_ivar_to_job(\"%s\", %s, &%s);" % (key, val, JOB_VAR_NAME) ), ivars.iteritems()))

    return format_code(job_config, indent)

def generate_cleanup_job(indent):
    code = [ "reprompib_cleanup_job(%s);" % (JOB_VAR_NAME) ]
    return format_code(code, indent)


def generate_start_measurement_loop(indent):
    spaces = ' ' * indent
#    init_job_code = generate_init_job(test_var, msize_var, indent)
    init_sync_code = generate_init_sync(indent)

    code = [ "for (%s = 0; %s < %s.n_rep; %s++) { " % (NREP_INDEX_VAR_NAME,
                                                       NREP_INDEX_VAR_NAME, PARSED_OPTS_VAR,
                                                       NREP_INDEX_VAR_NAME)
            ]
    return init_sync_code + format_code(code, indent)


def generate_stop_measurement_loop(indent):
    code = "}"
    return format_code(code, indent)



def generate_add_to_dictionary(dict, indent):
    spaces = ' ' * indent
    code = map((lambda t: "reprompib_add_element_to_dict(\"%s\", %s);" % (t, dict[t]) ), dict.keys())
    return format_code(code, indent)


def generate_set_variable(dict, indent):

    code = map((lambda t: "%s = strdup(%s);" % (t, dict[t]) ), dict.keys())
    return format_code(code, indent)



def format_code(code_lines, indent):
    spaces = ' ' * indent
    code = map((lambda t: "%s%s\n" % (spaces, t) ), code_lines)
    return "".join(code)

