package com.example.harsh.testcpp.gtest;

import android.os.AsyncTask;
import android.util.Log;

/**
 * Created by harsh on 12/8/17.
 */

public class GTestDriver {

    private static final String LOG_TAG = "GTestDriver";

    public static void scheduleGTests(final String gtestCmdLine) {
        Log.i(LOG_TAG, "Scheduling tests on a worker thread.");
        AsyncTask.THREAD_POOL_EXECUTOR.execute(new Runnable() {
            @Override
            public void run() {
                runGTests(gtestCmdLine);
            }
        });
    }

    public static void runGTests(final String gtestCmdLine) {
        Log.i(LOG_TAG, "runGTests executing for CmdLine:" + gtestCmdLine);
        GTestDriver g = new GTestDriver();
        try {
            String[] args = CommandLine.translate("GTestDriver " + gtestCmdLine);
            int result = g.runGTestsNative(args);
            Log.i(LOG_TAG, "runGTests returned:" + result);
        } catch (Exception ex) {
            Log.e(LOG_TAG, "Error running GTest. Ex:" + ex.toString());
        }
    }

    native int runGTestsNative(String[] gtestCmdLineArgs);
}
