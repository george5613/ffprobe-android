package com.george5613.probe.logic;

import android.content.Context;
import android.text.TextUtils;

import com.alibaba.fastjson.JSON;
import com.george5613.probe.VideoDuration;
import com.george5613.probe.VideoFormat;
import com.getkeepsafe.relinker.ReLinker;

import java.io.File;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/**
 * Created by George on 2017/6/15.
 */

public class ProbeLogic {

    private static final String TAG = ProbeLogic.class.getCanonicalName();

    private static boolean isProbeInit = false;
    private static ExecutorService sPool = new ThreadPoolExecutor(0, Integer.MAX_VALUE, 3L,
            TimeUnit.SECONDS,
            new SynchronousQueue<Runnable>());

    public static void init(final Context context) {
        ReLinker.loadLibrary(context, "ffmpeg", new ReLinker.LoadListener() {
            @Override
            public void success() {
                ReLinker.loadLibrary(context, "probe", new ReLinker.LoadListener() {
                    @Override
                    public void success() {
                        isProbeInit = true;
                    }

                    @Override
                    public void failure(Throwable t) {
                        isProbeInit = false;
                    }
                });
            }

            @Override
            public void failure(Throwable t) {
                isProbeInit = false;
            }
        });

    }

    public static boolean isProbeInit() {
        return isProbeInit;
    }

    public static VideoDuration probeVideoDuration(String url) {
        return probeVideoDuration(url, null, -1);
    }

    public static VideoDuration probeVideoDuration(String url, Map<String, String> headers) {
        return probeVideoDuration(url, headers, -1);
    }

    public static VideoDuration probeVideoDuration(String url, Map<String, String> headers,
            int timeoutSec) {
        if(!isProbeInit)
            return new VideoDuration();
        if(TextUtils.isEmpty(url))
            return new VideoDuration();
        if(url.startsWith("/")) {
            File file = new File(url);
            if(!file.exists())
                return new VideoDuration();
        }
        String header = null;
        if(headers != null && !headers.isEmpty()) {
            StringBuilder sb = new StringBuilder();
            for(Map.Entry<String, String> entry : headers.entrySet()) {
                sb.append(entry.getKey());
                sb.append(":");
                String value = entry.getValue();
                if(!TextUtils.isEmpty(value))
                    sb.append(entry.getValue());
                sb.append("\r\n");
                header = sb.toString();
            }
        }
        String probeResult;

        if(!TextUtils.isEmpty(header)) {
            String[] command = new String[] {
                    "-v",
                    "quiet",
                    "-print_format",
                    "json",
                    "-show_entries",
                    "format=duration,start_time",
                    "-headers",
                    header,
                    "-i",
                    url
            };
            probeResult = timeoutSec > 0 ?
                    probeWaitFor(command, timeoutSec) :
                    probeVideo(command);
        } else {
            String[] command = new String[] {
                    "-v",
                    "quiet",
                    "-print_format",
                    "json",
                    "-show_entries",
                    "format=duration,start_time",
                    "-i",
                    url
            };
            probeResult = timeoutSec > 0 ?
                    probeWaitFor(command, timeoutSec) :
                    probeVideo(command);
        }
        VideoDuration duration;
        try {
            duration = JSON.parseObject(probeResult, VideoDuration.class);
        } catch(Exception e) {
            duration = new VideoDuration();
            duration.mFormat = new VideoDuration.Format();
        }
        return duration;
    }

    public static VideoFormat probeVideoFormat(String url) {
        return probeVideoFormat(url, null);
    }

    public static VideoFormat probeVideoFormat(String url, Map<String, String> headers) {
        return probeVideoFormat(url, headers, -1);
    }

    public static VideoFormat probeVideoFormat(String url, Map<String, String> headers,
            int timeoutSec) {
        if(!isProbeInit)
            return new VideoFormat();
        if(TextUtils.isEmpty(url))
            return new VideoFormat();
        if(url.startsWith("/")) {
            File file = new File(url);
            if(!file.exists())
                return new VideoFormat();
        }
        String header = null;
        if(headers != null && !headers.isEmpty()) {
            StringBuilder sb = new StringBuilder();
            for(Map.Entry<String, String> entry : headers.entrySet()) {
                sb.append(entry.getKey());
                sb.append(":");
                String value = entry.getValue();
                if(!TextUtils.isEmpty(value))
                    sb.append(entry.getValue());
                sb.append("\r\n");
                header = sb.toString();
            }
        }
        String probeResult;
        if(!TextUtils.isEmpty(header)) {
            String[] command = new String[] {
                    "-v",
                    "quiet",
                    "-print_format",
                    "json",
                    "-show_entries",
                    "format=format_name",
                    "-headers",
                    header,
                    "-i",
                    url
            };
            probeResult = timeoutSec > 0 ?
                    probeWaitFor(command, timeoutSec) :
                    probeVideo(command);
        } else {
            String[] command = new String[] {
                    "-v",
                    "quiet",
                    "-print_format",
                    "json",
                    "-show_entries",
                    "format=format_name",
                    "-i",
                    url
            };
            probeResult = timeoutSec > 0 ?
                    probeWaitFor(command, timeoutSec) :
                    probeVideo(command);
        }
        VideoFormat format;
        try {
            format = JSON.parseObject(probeResult, VideoFormat.class);
        } catch(Exception e) {
            format = new VideoFormat();
            format.mFormat = new VideoFormat.Format();
        }
        return format;
    }

    private static String probeWaitFor(final String[] commands, int timeoutSec) {
        String output = null;
        try {
            if(isProbeInit) {
                Callable<String> call = new Callable<String>() {
                    public String call() throws Exception {
                        return probeVideo(commands);
                    }
                };
                Future<String> executeFuture;
                executeFuture = sPool.submit(call);
                output = executeFuture.get(timeoutSec, TimeUnit.SECONDS);
            }
        } catch(InterruptedException | ExecutionException | TimeoutException ignore) {
        }
        return output;
    }

    private static native String probeVideo(String[] command);
}
