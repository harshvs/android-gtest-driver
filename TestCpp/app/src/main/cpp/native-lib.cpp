#include <jni.h>
#include <string>
#include <cstdlib>
#include <unistd.h>

#include <iostream>
#include <iomanip>
#include <thread>
#include <unwind.h>
#include <dlfcn.h>

#include <sstream>
#include <android/log.h>

#include <cxxabi.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

#include "JSONPathFinder.h"

#include "gtest/gtest.h"

struct BacktraceState
{
    void** current;
    void** end;
};

static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context* context, void* arg)
{
    BacktraceState* state = static_cast<BacktraceState*>(arg);
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        if (state->current == state->end) {
            return _URC_END_OF_STACK;
        } else {
            *state->current++ = reinterpret_cast<void*>(pc);
        }
    }
    return _URC_NO_REASON;
}

size_t captureBacktrace(void** buffer, size_t max)
{
    BacktraceState state = {buffer, buffer + max};
    _Unwind_Backtrace(unwindCallback, &state);

    return state.current - buffer;
}

void dumpBacktrace(std::ostream& os, void** buffer, size_t count) {
    const char *demangled = nullptr;
    for (size_t idx = 0; idx < count; ++idx) {
        const void *addr = buffer[idx];
        const char *symbol = "";
         Dl_info info;
        if (dladdr(addr, &info) && info.dli_sname) {
            symbol = info.dli_sname;
            int status = 0;
            // demangled = abi::__cxa_demangle(symbol, 0, 0, &status);
            if (status) {
//                symbol = demangled;
            } else {
//                if(demangled) {
//                    delete demangled;
//                    demangled = nullptr;
//                }

            }
        }

        os << "  #" << std::setw(2) << idx << ": " << addr << "  " <<  symbol << "\n";
        if (demangled) {
            delete demangled;
            demangled = nullptr;
        }
    }
}

std::string testJson() {
    using namespace std;
    using namespace rapidjson;

     string data = "{\"menu\": {  \"id\": \"file\",  \"value\": \"File\",  \"popup\": {    \"menuitem\": [      {\"value\": \"New\", \"onclick\": \"CreateNewDoc()\"},      {\"value\": \"Open\", \"onclick\": \"OpenDoc()\"},      {\"value\": \"Close\", \"onclick\": \"CloseDoc()\"}    ]  }}}";
    Document d;
    d.Parse(data);

    Value& menu = d["menu"];

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    menu.Accept(writer);

    return buffer.GetString();
}

const char* cccc( JNIEnv* env) {
    const size_t max = 1024;
    void* buffer[max];
    std::ostringstream oss;
    dumpBacktrace(oss, buffer, captureBacktrace(buffer, max));

    // __android_log_print(ANDROID_LOG_INFO, "app_name", "%s", oss.str().c_str());
    // std::string hello = "Hello from Real C++";

    return oss.str().c_str();

}

const char* bbbb( JNIEnv* env) {
    return cccc(env);
}

const char* aaaa( JNIEnv* env) {
    return bbbb(env);
}

void pipeStdoutToLog() {
    using namespace std;
    __android_log_print(ANDROID_LOG_INFO, "GTEST_SETUP", "Setting up STDOUT pipe to adb log");
    int stdoutPipe[2];
    pipe(stdoutPipe);
    dup2(stdoutPipe[1], STDOUT_FILENO);
    FILE *exitEndFd = fdopen(stdoutPipe[0], "r");
    // char buffer[256];
    stringstream outStm;
    int c;
    while (true) {
        c = fgetc(exitEndFd);
        // __android_log_print(ANDROID_LOG_INFO, "APP_STDOUT", ">%c<", (char)c );
        if (c == '\n' || c == EOF) {
            __android_log_print(ANDROID_LOG_INFO, "APP_STDOUT", "%s", outStm.str().c_str());
            outStm.str("");
        } else {
            outStm << (char) c;
        }
        if (c == EOF) {
            break;
        }
        // fgets(buffer, sizeof(buffer), exitEndFd);
        // __android_log_print(ANDROID_LOG_INFO, "APP_STDOUT", "%s", buffer);
    }
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

//JNIEXPORT jint JNICALL
//Java_com_example_harsh_testcpp_GoogleTestDriver_runGTests(JNIEnv *env, jclass type, jobject args) {
//    // TODO
//    int argc = 0;
//    char **argv = 0;
//    ::testing::InitGoogleTest(&argc, argv);
//    int result = RUN_ALL_TESTS();
//    return result;
//}

//extern "C"
//JNIEXPORT jint JNICALL
//Java_com_example_harsh_testcpp_GoogleTestDriver_runGTests(JNIEnv *env, jclass type) {
//
//    // TODO
//    int argc = 2;
//    char **argv;
//    argv = new char*[2];
//    argv[0] = new char[32];
//    strcpy(argv[0], "TestApp");
//    argv[1] = new char[32];
//    strcpy(argv[1], "--gtest_list_tests");
//
//    ::testing::InitGoogleTest(&argc, argv);
//    fflush(stdout);
//    int result = RUN_ALL_TESTS();
//    fflush(stdout);
//    return result;
//
//}

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
    std::string runId(argv[argc-1]);
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


//extern "C"
//JNIEXPORT jint JNICALL
//Java_com_example_harsh_testcpp_GoogleTestDriver_runGTestsNative_old(JNIEnv *env,
//                                                                jobject instance,
//                                                                jstring gtestCmdLine_) {
//    const char *gtestCmdLine = env->GetStringUTFChars(gtestCmdLine_, 0);
//
//    int argc = 1;
//    char **argv = 0;
//
////    wordexp_t p;
////    std::stringstream stm;
////    stm<<"GTestDriver"<<" "<<"-l -t /etc \"hello harsh\" --gtest-color=yes --gtest-list-test=ok";
////
////    p.we_offs = 0;
////    wordexp(stm.str().c_str(), &p, WRDE_NOCMD);
////
////    for(int i=0; i < p.we_wordc; i++)
////    {
////        printf("%s\n", p.we_wordv[i]);
////    }
////
////    wordfree(&p);
//
//
//    argv = new char *[2];
//    argv[0] = new char[32];
//    strcpy(argv[0], "TestApp");
//    argv[1] = new char[32];
//    strcpy(argv[1], "--gtest_list_tests");
//
//    ::testing::InitGoogleTest(&argc, argv);
//    fflush(stdout);
//    int result = RUN_ALL_TESTS();
//    fflush(stdout);
//
//    env->ReleaseStringUTFChars(gtestCmdLine_, gtestCmdLine);
//    return result;
//}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_harsh_testcpp_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {

    using namespace std;

    gtestSetup();

    string jsonPath{"$.menu.id"};
    string doc{"{\"menu\": {  \"id\": \"file\",  \"value\": \"File\",  \"popup\": {    \"menuitem\": [      {\"value\": \"New\", \"onclick\": \"CreateNewDoc()\"},      {\"value\": \"Open\", \"onclick\": \"OpenDoc()\"},      {\"value\": \"Close\", \"onclick\": \"CloseDoc()\"}    ]  }}}"};

    // string doc = "{\\\"menu\\\": {  \\\"id\\\": \\\"file\\\",  \\\"value\\\": \\\"File\\\",  \\\"popup\\\": {    \\\"menuitem\\\": [      {\\\"value\\\": \\\"New\\\", \\\"onclick\\\": \\\"CreateNewDoc()\\\"},      {\\\"value\\\": \\\"Open\\\", \\\"onclick\\\": \\\"OpenDoc()\\\"},      {\\\"value\\\": \\\"Close\\\", \\\"onclick\\\": \\\"CloseDoc()\\\"}    ]  }}}";

    JSONPathFinder finder(jsonPath);

    stringstream result("Pointers:\n");
    for(const auto path : finder.GetPaths(doc)) {
        result << path << endl;
        // printf(">>>>>>>>>>>>>>>> %s\n", path.c_str());
    }
    fflush(stdout);

//    for(int i=0;i<500;i++) {
//        printf("---------------");
//    }

    // __android_log_print(ANDROID_LOG_INFO, "app_name", "%s", oss.str().c_str());
    // std::string hello = "Hello from Real C++";
    // return env->NewStringUTF(aaaa(env));
    return env->NewStringUTF(result.str().c_str());
}
