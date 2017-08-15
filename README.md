# android-gtest-driver
Google Test for NDK based Android Apps

Here we have a sample NDK based Android project and a python script to
show integration of the [Google
Tests]((https://github.com/google/googletest)) to run native (C++)
instrumented unit tests.

## Background & Rationale

The Unit testing of the shared native (C++) modules typically requires
a large overhead of extracting/maintaining/running the modules outside
of their real context so they could run under tests on the
development/test machines. This usually requires rebuilding the
binaries for the target machine with mocking and simulating things
around it.

In practice, real systems are complex with myriad
inter-dependencies. Testing a component by mocking its actual
dependencies and, that too, outside of its actual run-time context is
a complex, costly and tedious task. It is also hard to guarantee that
the module under test will behave in exactly the same way in both
test(dev box) and production environment. Also, it becomes
increasingly hard to maintain the pure unit testing with mock
everything strategy in presence of pressing deadlines and influx of
new features. Often the ball gets dropped in the middle of the project
and thereafter abandoned - as the cost (engineering debt) to revive it
then gets prohibitively expensive.

While pure Unit testing provides its value; instrumented unit tests
which run on actual device/emulator provide a sweet spot. It gets us
higher ROI and greater confidence in quality (as entities get tested
under real conditions). The downside of it could be slowness and
flakiness of the test runs. But those concerns can be mitigated by
avoiding delay causing steps like UI interactions and by providing
controlled network conditions (say by testing against services
available in the local network).

Using instrumented unit tests helps to reduce the effort to write and
maintain mock code. We are free to use a mocking framework, if we
choose, to simulate any dependency (say a flaky external service or
faking UI interaction).

A unit in this context can be as small or big as we choose or see
fit. The same test strategy therefore covers a wide specturm of
testing (Unit/Component/Integration). This reasonable approach can
potentially give us a far greater ROI and control over
writing/maintaing test cases.

## Problem

The tooling support for writing/maintaing instrumented C++ unit tests
seems absent for Android.  Here we may choose to hand-craft
instrumentation tests but that effort won't scale in comparision to
using an industry standard testing framework like Google Test. 

Google Test is a popular open sourced xUnit based C++ testing
framework from Google. It is put under use for testing large scale and
critial projects like Chormium, LLVM, Protocol Buffer and the Android
NDK etc. It is primarily command line based system, though its
integration in IDEs (VS, CLion) is supported. There are some open
source UI frontends to it are available too. Google Test is
light-weight, robust and having good eco-system/community - but it
lacks support when it comes to its integration in Android
projects. Android Studio does not provide any tooling support for it
either.

android-gtest-driver tries to fill that gap.

## Idea

Google Test invokes the tests for the system under test by parsing the
supplied command line parameters, and puts the result into its STDOUT
channel.

The idea is to introduce a proxy program which runs on the command
shell (on dev machine) which mimics the command line behavior of the
Google Tests running inside the App. The driver program behaves as if
it is the system under test, but the actual test initialization and
the test runs happen on the device/emulator in the context of the app
process.

This proxy way of interaction is achieved using ADB channel and by
redirection of the app process's STDOUT channel to android log. The
driver program keeps track of test results by scanning adb logs.

### Input to Google Test Run

The input from console to Google Test runtime inside the app is
handled by the following steps:

...1 Test driver (gtest.py) takes the test command line params.
...2 A broadcast intent (GTEST_CMD) with those parameters as intent extra is sent via ADB to the device
...3 The broadcast is received at the app's GTest Driver receiver.
...4 The test parameters are extracted from the intent.
...5 The native Google test run (a JNI call) is initiated by initializing gtest for the supplied parameters.

### Output from Google Test Run

The putput from the Google Test is delivered to console by the
following steps:

...1 The Google test runtime generates results on the app process's STDOUT channel.
...2 The STDOUT of the process is funnelled via a PIPE to a new FILE handle. 
...3 Input stream from the PIPE is put into Android logs (by a worker thread, AppStdoutPump).
...4 The ADB gets the logs from the device to the test driver (gtest.py).
...5 The driver monitors the start and the end of Google Test specific logs.
...6 The driver outputs the results to its own STDOUT channel.

## Design

![Android Google Test Driver](android_gtest_driver.png)
