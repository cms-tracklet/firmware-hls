#!/usr/bin/env python

import argparse
import os
import re
import pandas as pd

# Python 2-3 compatibility
try:
        import future, builtins, past, six
except ImportError as error:
        raise ImportError("Unable to import the python2/3 compatibility modules (future, builtins, past, six)")

def parse_reference_file(filename):
    events = []
    with open(filename,'r') as f:
        values = []
        for iline, line in enumerate(f):
            if line.startswith('BX = ') and iline == 0:
                continue
            elif line.startswith('BX = '):
                events.append(values)
                values = []
            else:
                values.append(line.split()[2].upper().replace('X','x'))
        events.append(values)
    return events

def get_column_index(layers, df):
    match = False
    column_index = -1
    for icol, column_header in enumerate(df.columns):
        match = len(re.findall(layers,column_header))>0
        if match:
            column_index = icol
            break
    return icol

def print_results(total_number_of_events, number_of_good_events, number_of_missing_events, number_of_event_length_mismatches, number_of_value_mismatches):
    print("\nResults\n"+(7*'='))
    print("Good events: "+str(number_of_good_events))
    print("Bad events: "+str(total_number_of_events-number_of_good_events))
    print("\tMissing events: "+str(number_of_missing_events))
    print("\tLength mismatches: "+str(number_of_event_length_mismatches))
    print("\tValue mismatches: "+str(number_of_value_mismatches)+"\n\n")

def compare_full_matches(comparison_filename="", fail_on_error=False, file_location='/', reference_filenames=[], verbose=False):
    # Sanity checks
    if len(reference_filenames) == 0:
        raise Exception("No reference files were specified (-r). At least one reference file is needed to run the comparison.")

    for reference_filename in reference_filenames:
        # Define some counters
        number_of_missing_events=0 # The number of events missing from the testbench output
        number_of_event_length_mismatches=0 # The number of events where the length of the testbench output doesn't match the length of the reference output
        number_of_value_mismatches=0 # The number of times the values between the testbench output and the reference output don't match
        number_of_good_events=0

        #Find the layer names for the reference file
        layers = re.search("(L[0-9]L[0-9])",reference_filename).group(0)
        print("Comparing the values for layers "+str(layers)+" to the reference file "+str(reference_filename)+" ... ")

        #Parse the reference data
        reference_data = parse_reference_file(file_location+"/"+reference_filename)

        # Read column names from file
        column_names = list(pd.read_csv(file_location+"/"+comparison_filename,delim_whitespace=True,nrows=1))
        column_names.insert(1,"unit")
        #column_names[2]=column_names[2][:-1]
        column_names[4]=column_names[4]+".0"
        column_names.insert(5,column_names[4]+".1")
        column_names.insert(10,column_names[9]+".1")
        if verbose: print(column_names)

        #Open the comparison data
        column_selections = ['BX#','enb','readaddr','FM_']
        data = pd.read_csv(file_location+"/"+comparison_filename,delim_whitespace=True,header=0,names=column_names,usecols=[i for i in column_names if any(select in i for select in column_selections)])
        if verbose: print(data) # Can also just do data.head()

        #Get the column index for the correct columns of data
        value_index = get_column_index(layers,data)
        address_index = value_index-1
        valid_index = value_index-2
        selected_columns = data[['BX#',data.columns[valid_index],data.columns[address_index],data.columns[value_index]]]

        group_index = 0
        group_sub_index = -1

        for ievent,event in enumerate(reference_data):
            print("Doing event "+str(ievent+1)+"/"+str(len(reference_data))+" ... ")
            good = True

            # Select the correct event from the comparison data
            selected_rows = selected_columns.loc[selected_columns['BX#'] == ievent]
            if len(selected_rows) == 0 and len(event) != 0:
                good = False
                number_of_missing_events += 1
                message = "Event "+str(ievent+1)+" does not exist in the comparison data!"
                if fail_on_error: raise Exception(message)
                else:             print("\t"+message)

            # Select only the comparison data where the valid bit is set
            selected_rows = selected_rows.loc[selected_rows[selected_rows.columns[1]] == '0b1']

            # Check the legnth of the two sets
            # Raise an exception if the are fewer entries for a given event in the comparison data than in the reference data
            if len(selected_rows) != len(event):
                good = False
                number_of_event_length_mismatches += 1
                message = "The number of entries in the comparison data doesn't match the number of entries in the reference data for event "+str(ievent+1)+"!"\
                          "\n\treference="+str(len(event))+" comparison="+str(len(selected_rows))
                if fail_on_error: raise Exception(message)
                else:             print("\t"+message.replace("\n","\n\t"))

            # If there are no entries in the comparison data, then don't try to compare any of the values
            if len(selected_rows) == 0: continue

            offset = selected_rows[selected_rows.columns[3]].index[0]
            for ival,val in enumerate(event):
                # In case there are fewer entries in the comparison data than in the reference data
                if offset+ival not in selected_rows[selected_rows.columns[3]]: continue

                # Raise exception if the values for a given entry don't match
                if selected_rows[selected_rows.columns[3]][offset+ival] != val:
                    good = False
                    number_of_value_mismatches += 1
                    message = "The values for event "+str(ievent)+" address "+str(selected_rows[selected_rows.columns[2]][offset+ival])+" do not match!"\
                              "\n\treference="+str(val)+" comparison="+str(selected_rows[selected_rows.columns[3]][offset+ival])
                    if fail_on_error: raise Exception(message)
                    else:             print("\t\t"+message.replace("\n","\n\t\t"))

            number_of_good_events += good

        print_results(len(reference_data),number_of_good_events,number_of_missing_events,number_of_event_length_mismatches,number_of_value_mismatches)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""
Compare reference files to the output of the testbench.

Dependencies:
=============
The main dependency is the module 'pandas', though to make sure this program is compatible both with python 2 and python 3 we also require the 'future', 'builtins', 'past', and 'six' modules.

Examples:
=========
python3 CompareFullMatches.py --help
python3 CompareFullMatches.py -l ~/Downloads/ -r FullMatches_FM_L1L2_L3PHIC_04.dat FullMatches_FM_L5L6_L3PHIC_04.dat
""",
                                    epilog="",
                                    formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-c","--comparison_filename",default="output.txt",help="The filename of the testbench output file (default = %(default)s)")
    parser.add_argument("-f","--fail_on_error",default=False,action="store_true",help="Raise an exception on the first error as opposed to simply printing a message (default = %(default)s)")
    parser.add_argument("-l","--file_location",default="./",help="Location of the input files (default = %(default)s)")
    parser.add_argument("-r","--reference_filenames",default=[],nargs="+",help="A list of filenames for the reference files (default = %(default)s)")
    parser.add_argument("-v","--verbose",default=False,action="store_true",help="Print extra information to the console (default = %(default)s)")

    args = parser.parse_args()

    compare_full_matches(**vars(args))
