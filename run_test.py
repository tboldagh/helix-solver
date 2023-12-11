#!/usr/bin/env python3

import os
import sys
import subprocess
from pathlib import Path


def check_environment():
    oneapi_setvars_completed = os.environ.get('SETVARS_COMPLETED')
    if oneapi_setvars_completed != '1':
        print('OneAPI environment has not been initialized. Source setvars.sh before running tests.')
        return False
    
    return True

def suites_list_element_to_suite(suites_list_element):
    suite = suites_list_element.rstrip()

    name = suite.split('/')[-1]

    path = f'build/{suite}'

    log = f'{path}.log'

    # In future we can add Valgrind or something here
    run_command = path
    
    return {
        'name': name,
        'path': path,
        'log': log,
        'run_command': run_command
    }


def run_suite(suite):
    try:
        result = subprocess.run(suite['run_command'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        return_code = result.returncode
        passed = return_code == 0
        stdout = result.stdout
        stderr = result.stderr

    except FileNotFoundError:
        return_code = None
        passed = False
        stdout = None
        stderr = None
        error_type = 'file_not_found'
        error = None

    except subprocess.CalledProcessError as e:
        return_code = None
        passed = False
        stdout = None
        stderr = None
        error_type = 'process_error'
        error = e

    else:
        error_type = None
        error = None

    return {
        'return_code': return_code,
        'passed': passed,
        'stdout': stdout,
        'stderr': stderr,
        'error_type': error_type,
        'error': error
    }


def get_suite_pass_fail_string(result):
    pass_fail_string = ''
    if result['passed']:
        pass_fail_string += 'PASSED'
        pass_fail_string += '\n' if result['stderr'] == '' else ' with STDERR\n'
    else:
        pass_fail_string += 'FAILED'
        if result['error_type'] is None:
            pass_fail_string += '\n'
        elif result['error_type'] == 'file_not_found':
            pass_fail_string += ' Executable not found\n'
        elif result['error_type'] == 'process_error':
            pass_fail_string += f' Process exited with code: {result["error"].returncode}\n'
        else:
            pass_fail_string += f' Process exited unknown error type: {result["error_type"]}\n'
    return pass_fail_string


def get_suite_log(suite):
    log = ''
    log += f'{suite["name"]} '
    log += get_suite_pass_fail_string(suite['result']) + '\n'

    if suite['result']['error_type'] is not None:
        log += f'Suite produced error of type: {suite["result"]["error_type"]}\n'
        if suite['result']['error'] is not None:
            log += f'{suite["result"]["error"]}\n'
        log += '\n'

    if suite['result']['stderr'] is None:
        log += 'Suite produced no STDERR output.\n\n'
    else:
        log += 'Suite produced following STDERR output:\n'
        log += suite['result']['stderr'] + '\n\n'

    if suite['result']['stdout'] is None:
        log += 'Suite produced no stdout output.\n\n'
    else:
        log += 'Suite produced following STDOUT output:\n'
        log += suite['result']['stdout'] + '\n\n'

    log += f'Log files are located in: {suite["log"]}\n'
    log += f'To reproduce run: {suite["run_command"]}\n'
    return log


def print_summary(suites, passed_suites, failed_suites, passed_with_stderr_suites):
    print('\n' + '#' * 79)
    print('#' * 20 + ' SUMMARY ' + '#' * 20)
    print('Suites:')
    print(f'\ttotal: {len(suites)}')
    print(f'\tpassed: {len(passed_suites)}')
    print(f'\tpassed with STDERR: {len(passed_with_stderr_suites)}')
    print(f'\tfailed: {len(failed_suites)}')
    print('')

    if len(failed_suites):
        print('Failed suites:')
        for suite in failed_suites:
            print(f'\t{suite["name"]} {get_suite_pass_fail_string(suite["result"])}', end='')
        print('')        

    if len(passed_with_stderr_suites):
        print('Suites passed with STDERR:')
        for suite in passed_with_stderr_suites:
            print(f'\t{suite["name"]} {get_suite_pass_fail_string(suite["result"])}', end='')
        print('')


def main():
    if not check_environment():
        print('Environment check failed. Environment is not ready to run tests.')
        return

    # TODO: load from sys.argv
    settings = {
        'save_stderr': True,
        'save_passed': True,
        'show_passed': True
    }

    test_suites_list_file_path = 'build/TestSuitesList.txt'
    with open(test_suites_list_file_path, 'r') as test_suites_list_file:
        test_suites_list = test_suites_list_file.readlines()

    suites = [suites_list_element_to_suite(i) for i in test_suites_list]

    passed_suites = []
    failed_suites = []
    passed_with_stderr_suites = []

    for suite in suites:
        print(f'Running {suite["name"]} ... ', end='')

        result = run_suite(suite)
        suite['result'] = result

        if result['passed']:
            if result['stderr'] == '':
                passed_suites.append(suite)
            else:
                passed_with_stderr_suites.append(suite)
        else:
            failed_suites.append(suite)

        print(get_suite_pass_fail_string(result), end='')

        log = get_suite_log(suite)

        if not result['passed'] or suite['result']['stderr'] != '' or settings['show_passed']:
            print('\n' + '#' * 79)
            print(log, end='')
            print('#' * 79 + '\n')

        if not result['passed'] or (settings['save_stderr'] and suite['result']['stderr'] != '') or settings['save_passed']:
            Path(os.path.dirname(suite['log'])).mkdir(parents=True, exist_ok=True)
            with open(suite['log'], 'w+') as log_file:
                log_file.write(log)

    print_summary(suites, passed_suites, failed_suites, passed_with_stderr_suites)

if __name__ == '__main__':
    main()