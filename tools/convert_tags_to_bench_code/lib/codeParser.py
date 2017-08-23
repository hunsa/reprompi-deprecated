import sys
import os
import re
import pprint
import errno
from __builtin__ import len
from bench_functions_gen import *

RUNTIME_COMPUTATION_METHODS = ["all", "reduce"]
RUNTIME_COMPUTATION_REDUCTION_OPS = ["min", "max", "mean"]

class CodeParser:

    code_generators = { "init_sync" : generate_init_sync,
                "start_sync" : generate_start_sync,
                "stop_sync" : generate_stop_sync,
                "measure_timestamp" : generate_measure_timestamp,
                "initialize_bench" : generate_init_bench,
                "cleanup_bench" : generate_cleanup_bench,
                "cleanup_sync" : generate_cleanup_sync,
                "print_runtime_array" : generate_print_output,
                "start_measurement_loop" : generate_start_measurement_loop,
                "stop_measurement_loop" : generate_stop_measurement_loop,
                "initialize_timestamps": generate_init_timestamp_array,
                "declare_variables" : generate_declare_variables,
                "cleanup_variables": generate_cleanup_variables,
                "add_includes" : generate_add_includes,
                "set" : generate_set_variable,
                "global" : generate_add_to_dictionary
                }


    #//@ initialize_timestamps t2
    #//@ initialize_timestamps t3
    #//@initialize_runtime_list name=runtime_coll end_time=t2 start_time=t1 type=reduce op=max
    #//@initialize_runtime_list name=total_runtime end_time=t3 start_time=t1 type=all

    #//@ start_measurement_loop test_var=meas_func msize=

    #//@ start_sync
    #//@ measure_timestamp t1


    def __init__(self, input_file, output_dir):
        self.input_file = input_file
        self.output_dir = output_dir

        self.annotations = {}
        self.ts_arrays = {}
        self.strings_array = {}
        self.output_config_list = {}



    def parse_line(self, line):
        tag = {}
        m = re.search('(?P<indent>[\s\t]*)//@([\s\t]*)(?P<keyword>[a-zA-Z0-9_]+)(?P<info>.*)', line)
        if (m != None):
            keyword = m.group('keyword')
            tag["keyword"] = keyword
            data = {}
            indent = m.group('indent')
            if (indent != None):
                tag['indent'] = len(indent)
            else:
                tag['indent'] = 0

            info = m.group('info')
            while (info and len(info) > 0):
                m = re.search('(?P<key>[a-zA-Z0-9_]+)(=?)(?P<value>[^\s]*)(?P<info>.*)', info)
                if (m != None):
                    key = m.group('key')
                    value = m.group('value')
                    data[key] = value
                    info = m.group('info')
                else:
                    info = None
            tag["data"] = data
            print tag
        return tag


    def parse_file(self):
        line_no = 1
        keywords = self.code_generators.keys()
        self.annotations[self.input_file] = []
        with open(self.input_file) as f:
            for line in f:
                tag = self.parse_line(line)

                try:
                    if tag["keyword"] in keywords:
                        tag["line"] = line
                        tag["line_no"] = line_no

                        self.annotations[self.input_file].append(tag)

                except: # no keyword found
                    pass
                line_no = line_no + 1


    def process_data(self):
        for file_name, annotation_list in self.annotations.items():

            self.ts_arrays[file_name] = []
            self.strings_array[file_name] = []

            for annotation in annotation_list:
                if annotation['keyword'] == "initialize_timestamps":
                    data = annotation['data']
                    for key in data.keys():
                        if data[key] == "": # found key corresponding to the timer's name
                            self.ts_arrays[file_name].append(key)


                if annotation['keyword'] == "set":
                    data = annotation['data']
                    for key in data.keys():
                        self.strings_array[file_name].append(key)



        self.output_config_list = {}
        for file_name, annotation_list in self.annotations.items():
            for annotation in annotation_list:
                if annotation['keyword'] == "print_runtime_array":

#                    print "\n\n>>>>>>>>>>>>>>>>>>>>"
#                     print file_name
#                    print annotation
#                    print ">>>>>>>>>><<<<<<<<<<"

                    data = annotation['data']
                    output_config = {}
                    output_config["string_list"] = {}
                    output_config["int_list"] = {}

                    for key in data.keys():
                        if key == "start_time":
                            output_config["start_time"] = data["start_time"]
                        elif key == "end_time":
                            output_config["end_time"] = data["end_time"]
                        elif key == "name":
                            output_config["name"] = data["name"]
                        elif key == "type":
                            assert (data["type"] in RUNTIME_COMPUTATION_METHODS)
                            output_config["type"] = data["type"]
                        elif key == "op":
                            assert (data["op"] in RUNTIME_COMPUTATION_REDUCTION_OPS)
                            output_config["op"] = data["op"]

                        else:   # user-defined variables
                            if data[key] in self.strings_array[file_name]:     # if defined with a "set" tag -> add as string
                                output_config["string_list"][key] = data[key]
                            else :      # not defined with a "set" tag -> add as int
                                # NOTE: here we should check that the int variable is actually defined !!!!!!!
                                output_config["int_list"][key] = data[key]

                    try:
                        output_config["start_time"]
                    except KeyError:
                        print("Incorrect \"print_runtime_array\" specification (missing start_time)")
                        exit(1)
                    try:
                        output_config["end_time"]
                    except KeyError:
                        print("Incorrect \"print_runtime_array\" specification (missing end_time)")
                        exit(1)
                    try:
                        output_config["name"]
                    except KeyError:
                        print("Incorrect \"print_runtime_array\" specification (missing name)")
                        exit(1)
                    try:
                        output_config["type"]
                    except KeyError:
                        print("Incorrect \"print_runtime_array\" specification (missing type)")
                        exit(1)

                    if output_config["type"] == "reduce":
                        try:
                            output_config["op"]
                        except KeyError:
                            print("Warning: incomplete \"print_runtime_array\" specification (missing op). Using op=max")
                            output_config["op"] = "max"
                    else:
                        output_config["op"] = ""


                    if not self.output_config_list.has_key(file_name):
                        self.output_config_list[file_name] = {}

                    self.output_config_list[file_name][annotation["line_no"]] = [ output_config ]
                    print output_config



    def generate_output_file(self):

        try:    # make sure the output directory exists
            os.makedirs(self.output_dir)
        except OSError as e:
            if e.errno == errno.EEXIST and os.path.isdir(self.output_dir):
                pass
            else:
                raise

        filename = os.path.basename(self.input_file)
        output_file = os.path.join(self.output_dir, filename)

        ann_index = 0
        current_annotation = None

        self.process_data()

        with open(output_file, "w") as outf:
            line_no = 1
            with open(self.input_file) as f:
                for line in f:
                    outf.write(line)

                    if current_annotation == None and ann_index < len(self.annotations[self.input_file]):
                        current_annotation = self.annotations[self.input_file][ann_index]
                        ann_index = ann_index + 1

                    # insert code
                    if current_annotation != None and line_no == current_annotation["line_no"]:
                        if current_annotation["keyword"] == "print_runtime_array":
                            output_config_l = self.output_config_list[self.input_file][line_no]
                            for output_config in output_config_l:
                                if output_config["name"] == current_annotation['data']['name']:
                                    code = self.code_generators[current_annotation["keyword"]](output_config,
                                                                    current_annotation['indent'])


                        elif current_annotation["keyword"] in ["initialize_timestamps",
                                                               "measure_timestamp"
                                                               ]:
                            ts_name = ""
                            for key in current_annotation["data"].keys():
                                if current_annotation["data"][key] == "": # found key corresponding to the timer's name
                                    ts_name = key
                                    break
                            if len(ts_name) > 0:
                                code = self.code_generators[current_annotation["keyword"]](ts_name,
                                                                                           current_annotation['indent'])
                            else:
                                print("Error: incorrect annotation - %s" % current_annotation["line"])
                                exit(1)

                        elif current_annotation["keyword"] == "stop_measurement_loop":
                            code = self.code_generators[current_annotation["keyword"]](current_annotation['indent'])

                        elif current_annotation["keyword"] == "declare_variables":
                            ann_list = self.annotations[self.input_file]

                            main_file = False
                            for a in ann_list:
                                if a["keyword"] == "initialize_bench":
                                    main_file = True
                                    break

                            code = self.code_generators[current_annotation["keyword"]](main_file, self.ts_arrays[self.input_file],
                                                                                       self.strings_array[self.input_file],
                                                                                       current_annotation['indent'])

                        elif current_annotation["keyword"] == "cleanup_variables":
                            code = self.code_generators[current_annotation["keyword"]](self.ts_arrays[self.input_file],
                                                                                       self.strings_array[self.input_file],
                                                                                         current_annotation['indent'])

                        elif current_annotation["keyword"] in ["global",
                                                               "set"]:
                            code = self.code_generators[current_annotation["keyword"]](current_annotation['data'],
                                                                                         current_annotation['indent'])

                        elif current_annotation["keyword"] == "start_measurement_loop":
                            code = self.code_generators[current_annotation["keyword"]](current_annotation['indent'])
#                             try:
#                                 test_var = current_annotation["data"]["test_var"]
#                                 if len(test_var) == 0:
#                                     test_var = "\"test\""
#                             except:
#                                 print("Error: incorrect annotation - %s" % current_annotation["line"])
#                                 exit(1)
#
#                             try:
#                                 msize_var = current_annotation["data"]["msize_var"]
#                                 if len(msize_var) == 0:
#                                     msize_var = "0"
#                             except:
#                                 print("Error: incorrect annotation - %s" % current_annotation["line"])
#                                 exit(1)
#                            code = self.code_generators[current_annotation["keyword"]](test_Var, msize_var, current_annotation['indent'])

                        else:
                            code = self.code_generators[current_annotation["keyword"]](current_annotation['indent'])
                        outf.write(code)
                        current_annotation = None

                    line_no = line_no + 1




