from subprocess import *
from sys import exit
import os
import shutil

# Kills the server before exiting with given return value
def exit_test(server, return_val):
    server.kill()
    exit(return_val)

# Checks output and exits test if check fails
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

    ## CREATE CONFIG ###
    print('Creating example config...')
    config_file_name = 'integration_test_config'
    config_file = open(config_file_name,'w')
    config_file.write("""
        port 8080;

        path /static StaticFileHandler {
          root integration_test_files/;
        }

        path /echo EchoHandler {}

        # Default response handler if no handlers match.
        default NotFoundHandler {}
        """)
    config_file.close()

    print('Starting Webserver...')
    server = Popen(['./Webserver', config_file_name], stdout=DEVNULL)

    ### ECHO HANDLER ###
    print('Sending echo request...')
    out = check_output(["curl", "-s", "localhost:8080/echo"])
    expected = b'GET /echo HTTP/1.1\r\nUser-Agent: curl/7.35.0\r\nHost: localhost:8080\r\nAccept: */*\r\n\r\n';

    print('Checking echo output...')
    expected_output(out, expected, server)

    ### STATIC FILE HANDLER ###
    print('Creating example static files...')
    #if not(os.path.exists('integration_test_files/')):
    os.makedirs('integration_test_files/', exist_ok=True)
    path = 'integration_test_files/asdf.txt'
    test_file = open(path,'w')
    test_file.write('asdf')
    test_file.close()

    print('Sending file request...')
    out = check_output(["curl", "-s", "localhost:8080/static/asdf.txt"])
    expected = b'asdf'

    print('Checking file output...')
    expected_output(out, expected, server)

    ### 404 HANDLER ###
    print('Sending nonexistent file request...')
    out = check_output(["curl", "-s", "localhost:8080/static/does_not_exist.txt"])
    expected = b'<html><body><h1>404 Not Found</h1></body></html>'

    print('Checking response output...')
    expected_output(out, expected, server)

    ### MULTITHREADING ###
    print('Opening connection and sending file request...')
    Popen(['nc', 'localhost', '8080'], stdout=DEVNULL)
    out = check_output(["curl", "-s", "localhost:8080/static/asdf.txt"])
    expected = b'asdf'

    print('Checking response output...')
    expected_output(out, expected, server)

    print('Cleaning up...')
    os.remove(config_file_name)
    shutil.rmtree('integration_test_files')

    print('SUCCESS')
    exit_test(server, 0)

if __name__ == "__main__":
    main()