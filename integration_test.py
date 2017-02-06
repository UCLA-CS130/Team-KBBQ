from subprocess import *
from sys import exit
import os

def exit_test(server, return_val):
    server.kill()
    exit(return_val)

def expected_output(output, expected, server):
    if (output != expected):
        print('FAILED: Output does not match expected')
        print('Expected: ' + expected.decode('utf-8'))
        print('Output: ' + output.decode('utf-8'))
        exit_test(server, 1)

def main():
    print('Building Webserver...')
    if (call(['make', 'clean'], stdout=DEVNULL) != 0 or call(['make', 'Webserver'], stdout=DEVNULL) !=0):
        print('FAIL: Could not build Webserver.')
        return 1

    print('Starting Webserver...')
    server = Popen(['./Webserver', 'config'], stdout=DEVNULL)

    print('Sending echo request...')
    out = check_output(["curl", "-s", "localhost:8080/echo_request"])
    expected = b'GET /echo_request HTTP/1.1\r\nUser-Agent: curl/7.35.0\r\nHost: localhost:8080\r\nAccept: */*\r\n\r\n';

    print('Checking echo output...')
    expected_output(out, expected, server)

    if not os.path.exists('example_files'):
        os.makedirs('example_files')

    path = 'example_files/asdf.txt'
    fh = open(path,'w')
    fh.write('asdf')
    fh.close();

    print('Sending file request...')
    out = check_output(["curl", "-s", "localhost:8080/static/asdf.txt"])
    expected = b'asdf';

    print('Checking file output...')
    expected_output(out, expected, server)

    print('SUCCESS')
    exit_test(server, 0)

if __name__ == "__main__":
    main()