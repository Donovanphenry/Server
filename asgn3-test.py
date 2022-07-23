# Made by Jess Fan

import os
import time
import random
import requests
import subprocess
from concurrent.futures import ProcessPoolExecutor

subprocess.call(['sh', './init-test.sh'])
test_results=[]

class dispatcher():
    def __init__(self):
        self.port=1023
        self.backend_ports=[]
        self.programs={}
        self.dispatch_backends(4)
    
    def dispatch(self, args, quiet=True):
        with open(os.devnull, 'w') as fp:
            while True:
                self.port+=1
                cmd=["./httpproxy"]
                cmd.append(str(self.port))
                cmd.extend(args)
                cmd.extend(self.backend_ports)

                if quiet: process=subprocess.Popen(cmd, stdout=fp, stderr=fp)
                else: process=subprocess.Popen(cmd)
                time.sleep(0.1)
                if process.poll() is None: 
                    self.programs[self.port]=process
                    print(f"~~~~New httpproxy using command: {' '.join(cmd)}~~~~")
                    break
        return self.port

    def dispatch_backends(self, num):
        for i in range(num):
            while True:
                try: os.remove(f"log_file{i}")
                except: pass
                port=random.randint(8000, 8050)
                cmd=["./httpserver", "-l", f"log_file{i}"]
                cmd.append(str(port))

                process=subprocess.Popen(cmd)
                time.sleep(0.1)
                if process.poll() is None: 
                    self.programs[port]=process
                    self.backend_ports.append(str(port))
                    break
        print(f"~~~~Created backends {', '.join(self.backend_ports)}~~~~")

    def kill_program(self, port):
        print(f"~~~~Kill httpproxy On {port}~~~~")
        self.programs[port].kill

    def kill_all(self):
        subprocess.Popen(["pkill", "-f", "./httpserver*|./httpproxy*"])

def get_file(i, port, alt_data="Not Found\n"):
    try: data=open(f"r{i}.txt", "rb").read()
    except: data=alt_data.encode()
    try: r=requests.get(f"http://0.0.0.0:{port}/r{i}.txt")
    except Exception as e: 
        print(e)
        return False
    if r.content != data: return False
    else: return True

def read_logs():
    ret={}
    init=[open(f"log_file{i}", "r").read() for i in range(0,4)]
    for i in range(len(init)):
        ret[i]={"errors":0, "total":0}
        x = init[i].split('\n')

        print(f"x = {x}")
        for line in init[i].split("\n"):
            #ignore HEAD
            if "HEAD" not in line:
                if line.startswith("FAIL"):
                  ret[i]["errors"]+=1
                elif "healthcheck" not in line: ret[i]["total"]+=1
    return ret

def compare_logs(log1, log2):
    return [log1[i]!=log2[i] for i in range(len(log1))]

def sequence_logs(i, j, port):
    l0=read_logs()
    print(f"i = {i}, j = {j}")
    print(f"l0 = {l0}")
    if get_file(i, port):
        l1=read_logs()
        print(f"l1 = {l1}")
        if get_file(j, port):
            l2=read_logs()
            print(f"l2 = {l2}")
            return (compare_logs(l0,l1), compare_logs(l1,l2))

disp=dispatcher()

#print("====Simple GET Requests====")
#call_to=disp.dispatch(["-m", "1000000", "-N", "5"])
#for i in range(0,8):
#    run=get_file(i, call_to)
#    if run:  print(f"Test {len(test_results)}: GET r{i}.txt: PASS")
#    else: print(f"Test {len(test_results)}: GET r{i}.txt: FAIL")
#    test_results.append(run)

#print("====Not GET requests====")
#for i in range(10,18):
#    run=get_file(i, call_to)
#    if run:  print(f"Test {len(test_results)}: GET r{i}.txt: PASS")
#    else: print(f"Test {len(test_results)}: GET r{i}.txt: FAIL")
#    test_results.append(run)

#print("====Multithreaded GET Requests====")
#for i in range(1, 6):
#    with ProcessPoolExecutor(max_workers=i*2) as executor: #processpool becasue python threading isn't true multithreading due to the global interpreter lock
#        run=list(executor.map(get_file, range(0,18), [call_to]*len(range(0,18))))
#        if sum(run) < 8: print(f"Test {len(test_results)}: Multithreaded GET with {i*2} workers: FAIL")
#        else: print(f"Test {len(test_results)}: Multithreaded GET with {i*2} workers: PASS")
#        test_results.append(sum(run))
#disp.kill_program(call_to)

print("====Testing Cache====")
call_to=disp.dispatch(["-m", "1000000", "-N", "5", "-s", "3"])

#run=sequence_logs(0,0, call_to)
#if sum(run[0]) == 1 and sum(run[1]) == 0: print(f"Test {len(test_results)}: Test if a single file is cached: PASS")
#else: print(f"Test {len(test_results)}: Test that files bigger than -m are not cached: FAIL")
#test_results.append(sum(run[0]) == 1 and sum(run[1]) == 0)

run=sequence_logs(1,1, call_to)
if (sum(run[0]) == 1 and sum(run[1]) == 1) and (run[0].index(True) != run[1].index(True)): print(f"Test {len(test_results)}: Test if cache ignores files that are too large: PASS")
else: print(f"Test {len(test_results)}: Test if cache ignores files that are too large: FAIL. run = {run}")
test_results.append(sum(run[0]) == 1 and sum(run[1]) == 1) and (run[0].index(True) != run[1].index(True))

#run=sequence_logs(100,100, call_to)
#if sum(run[0]) == 1 and sum(run[1]) == 0: print(f"Test {len(test_results)}: Test if a 404 is cached: PASS")
#else: print(f"Test {len(test_results)}: Test if a 404 is cached: FAIL")
#test_results.append(sum(run[0]) == 1 and sum(run[1]) == 0)

#overrite first cache
#sequence_logs(200,200, call_to)
#sequence_logs(300,300, call_to)
#run=sequence_logs(0,0, call_to)
#if sum(run[0]) == 1 and sum(run[1]) == 0: print(f"Test {len(test_results)}: Test if cache is FIFO: PASS")
#else: print(f"Test {len(test_results)}: Test if cache is FIFO: FAIL")
#test_results.append(sum(run[0]) == 1 and sum(run[1]) == 0)
disp.kill_program(call_to)

print("====Testing Optional Functionality====")
#call_to=disp.dispatch(["-m", "1000000", "-N", "5", "-s", "3", "-u"])

# NOTE: If you're confident that your code works and don't know why you're failing this test,
#       consider the following: this test sends requests as fast as python is able to. Your
#       progam has multithreading and timestamps aren't infinitely precise; what should LRU
#       do when two timestamps are equal?
#sequence_logs(0,0, call_to)
#sequence_logs(100,100, call_to)
#sequence_logs(0,0, call_to)

#overrite first cache
#sequence_logs(200,200, call_to)
#sequence_logs(300,300, call_to)
#run=sequence_logs(0,0, call_to)
#if sum(run[0]) == 0 and sum(run[1]) == 0: print(f"Test {len(test_results)}: Test if cache is LRU: PASS")
#else: print(f"Test {len(test_results)}: Test if cache is LRU: FAIL")
#test_results.append(sum(run[0]) == 1 and sum(run[1]) == 0)
#disp.kill_program(call_to)


disp.kill_all()
