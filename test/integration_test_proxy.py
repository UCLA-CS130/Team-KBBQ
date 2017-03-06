#Referenced https://github.com/UCLA-CS130/TeamHello/blob/master/integration_test_proxy.py

import subprocess
import os
import time

FNULL = open(os.devnull, 'w')
subprocess.Popen(['make', 'Webserver'], stdout=subprocess.PIPE, cwd="./")
my_server = subprocess.Popen(['./Webserver', './config'], stdout=FNULL)

#Wait
time.sleep(0.02)

#Make requests to the proxy server and destination server
curl_proc = subprocess.call(["curl", "-s", "localhost:2020", "-o", "out1"])
curl_proc = subprocess.call(["curl", "-s", "www.ucla.edu", "-o", "out2"])

#calculate the difference for the two response 
diff_proc = subprocess.Popen(["diff", "-u", "out1", "out2"], stdout=subprocess.PIPE)
diff = diff_proc.stdout.read().decode('utf-8')

my_server.kill()
os.remove('out1')
os.remove('out2')

if diff != '':
    print("Proxy server integration test failed.")
    print(diff)
    exit(1)
print("Proxy server integration test passed.")
exit(0)