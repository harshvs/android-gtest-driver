#include <jni.h>
#include <string>
#include <cstdlib>
#include <unistd.h>

#include <iostream>
#include <iomanip>
#include <thread>

#include <sstream>
#include <android/log.h>

#include "gtest/gtest.h"

void pipeStdoutToLog() {
    using namespace std;
    __android_log_print(ANDROID_LOG_INFO, "GTEST_SETUP", "Setting up STDOUT pipe to adb log");
    int stdoutPipe[2];
    pipe(stdoutPipe);
    dup2(stdoutPipe[1], STDOUT_FILENO);
    FILE *exitEndFd = fdopen(stdoutPipe[0], "r");
    stringstream outStm;
    int c;
    while (true) {
        c = fgetc(exitEndFd);
        if (c == '\n' || c == EOF) {
            __android_log_print(ANDROID_LOG_INFO, "APP_STDOUT", "%s", outStm.str().c_str());
            outStm.str("");
        } else {
            outStm << (char) c;
        }
        if (c == EOF) {
            break;
        }
    }
    // TODO - close file handles.
}

volatile bool isPipeSetup = false;
std::thread* stdoutPump = nullptr;

void gtestSetup() {
    if(!isPipeSetup) {
        isPipeSetup = true;
        stdoutPump = new std::thread(pipeStdoutToLog);
        usleep(400);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_harsh_testcpp_GTestDriver_runGTestsNative(JNIEnv *env,
                                                           jobject instance,
                                                           jobjectArray gtestCmdLineArgs) {
    int argc = env->GetArrayLength(gtestCmdLineArgs);
    char **argv = new char *[argc + 1];

    for (int i = 0; i < argc; i++) {
        jstring string = (jstring) (env->GetObjectArrayElement(gtestCmdLineArgs, i));
        const char *rawString = env->GetStringUTFChars(string, 0);
        argv[i] = new char[strlen(rawString) + 1];
        strcpy(argv[i], rawString);
        env->ReleaseStringUTFChars(string, rawString);
    }

    gtestSetup();
    fflush(stdout);
    std::string runId(argv[argc - 1]);
    printf("GTest_Start:%s\n", runId.c_str());
    ::testing::InitGoogleTest(&argc, argv);
    fflush(stdout);
    int result = RUN_ALL_TESTS();
    printf("GTest_End:%s\n", runId.c_str());
    fflush(stdout);

    for (int i = 0; i < argc; i++) delete[] argv[i];
    delete[] argv;

    return result;
}
