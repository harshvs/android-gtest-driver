import os
import sys
import time
import uuid
import subprocess
from subprocess import call

runId = str(uuid.uuid4())
packageName = "com.example.harsh.testcpp"
gtestDriverIntentName = "com.example.harsh.testcpp.intent.GTEST_CMD"

def scanLogcat():
    adbCmd = "adb logcat | grep APP_STDOUT"
    # p = subprocess.Popen(adbCmd, stdout=subprocess.PIPE, bufsize=0, shell=True)
    p = subprocess.Popen(["adb", "logcat"], stdout=subprocess.PIPE, bufsize=0, shell=False)
    startMark = "GTest_Start:" + runId
    endMark = "GTest_End:" + runId
    gtestLogMark = "APP_STDOUT"
    while True:
        line = p.stdout.readline()
        if startMark in line:
            break
    while True:
        # print "."
        line = p.stdout.readline()
        if gtestLogMark not in line:
            continue
        if endMark in line:
            break
        if "referenceTable " in line:
            continue
        # Skip log prefix and newline
        line = line[line.index(":") +2 :-1]
        print line
        sys.stdout.flush()

# call(["adb", "logcat","-c"])

FNULL = open(os.devnull, 'w')
call(["adb","shell", "am", "force-stop", packageName], stdout=FNULL, stderr=subprocess.STDOUT)
call(["adb", "shell", "monkey", "-p",packageName, "1"], stdout=FNULL, stderr=subprocess.STDOUT)
time.sleep(1)
call(["adb","shell", "am", "broadcast", "-a", gtestDriverIntentName, "--es", "cmd", ' '.join(sys.argv) + " " + runId], stdout=FNULL, stderr=subprocess.STDOUT)
scanLogcat()


