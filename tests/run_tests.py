#! /usr/bin/env python3

from os import path, listdir
import sys
import json
import subprocess

cases_total = 0
cases_success = 0
directory = path.dirname(path.abspath(__file__))
print('Running in directory: ' + directory)

test_dirs = [file for file in listdir(directory) if path.isdir(path.join(directory, file))]
test_dirs.sort()
print('Detected ' + str(len(test_dirs)) + ' test configurations.')

def exec(cmd):
    if subprocess.call(cmd) != 0:
        raise ValueError("Failed to start subprocess: " + str(cmd))

def prepare_binary(test_dir, data):
    context = path.join(directory, test_dir)

    input_file = data['file']
    (name, extension) = path.splitext(input_file)

    if extension == '.nja':
        exec([path.join(directory, 'njc'), '--output', path.join(context, name + '.asm'), path.join(context, input_file)])

    exec([path.join(directory, 'nja'), path.join(context, name + '.asm'), path.join(context, name + '.bin')])
    return path.join(context, name + '.bin')

def run_test(file, input_config):
    if not isinstance(input_config, list):
        input_config = [input_config]
    input_config = [str(line) for line in input_config]
    input_text = ' '.join(input_config)

    refprocess = subprocess.Popen([path.join(directory, 'refnjvm'), file], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    refresult = refprocess.communicate(input=input_text.encode('utf-8'))[0]

    myprocess = subprocess.Popen(['./njvm', file], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    myresult = myprocess.communicate(input=input_text.encode('utf-8'))[0]

    if myresult != refresult:
        print('Expected:')
        print(str(refresult))
        print('Actual result:')
        print(str(myresult))
        return False
    return True



for test in test_dirs:
    try:
        print('Preparing test case: ' + test)
        with open(path.join(directory, test, 'data')) as config_file:
            data = json.load(config_file)
            bin_file = prepare_binary(test, data)
            for input_config in data['input']:
                print('===================== Test Case ' + str(1 + cases_total) + ' =====================')
                print('File: ' + bin_file)
                cases_total += 1
                if run_test(bin_file, input_config):
                    cases_success += 1
    except Exception as e:
        print("Error: " + str(e))
        cases_total += 1

print('Passed ' + str(cases_success) + '/' + str(cases_total) + ' tests!')
if cases_total > cases_success:
    sys.exit(1)
sys.exit(0)
