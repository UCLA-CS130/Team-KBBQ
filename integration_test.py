from subprocess import *
from sys import exit

def exit_test(server, return_val):
    server.kill()
    exit(return_val) 

def main():
    print('Building Webserver...')
    if (call(['make', 'clean'], stdout=DEVNULL) != 0 or call(['make', 'Webserver'], stdout=DEVNULL) !=0):
        print('FAIL: Could not build Webserver.')
        return 1

    print('Starting Webserver...')
    server = Popen(['./Webserver', 'config'], stdout=DEVNULL)

    print('Sending request...')
    out = check_output(["curl", "-s", "localhost:8080"])
    expected = b'GET / HTTP/1.1\r\nUser-Agent: curl/7.35.0\r\nHost: localhost:8080\r\nAccept: */*\r\n\r\n';

    print('Checking output...')
    if (out != expected):
        print('FAILED: Output does not match expected')
        print('Expected: ' + expected.decode('utf-8'))
        print('Output: ' + out.decode('utf-8'))
        exit_test(server, 1)

    print('SUCCESS')
    exit_test(server, 0)

if __name__ == "__main__":
    main()