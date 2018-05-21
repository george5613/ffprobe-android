package com.george5613.probe.example;

import android.app.ProgressDialog;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.Toast;

import com.george5613.probe.VideoDuration;
import com.george5613.probe.VideoFormat;
import com.george5613.probe.logic.ProbeLogic;

/**
 * A placeholder fragment containing a simple view.
 * <p>
 * HLS - Apple HTTP live streaming - m3u8
 * http://devimages.apple.com/iphone/samples/bipbop/bipbopall.m3u8
 * http://devimages.apple.com/iphone/samples/bipbop/gear1/prog_index.m3u8
 * http://live.3gv.ifeng.com/zixun.m3u8 (直播)
 * <p>
 * HTTP
 * http://www.modrails.com/videos/passenger_nginx.mov
 * http://wsmp32.bbc.co.uk/
 * <p>
 * RTSP
 * http://m.livestream.com (site)
 * rtsp://xgrammyawardsx.is.livestream-api.com/livestreamiphone/grammyawards
 */


public class MainActivityFragment extends Fragment {

    private static final int MESSAGE_SHOW_PROGRESS = 0x00;
    private static final int MESSAGE_DISMISS_PROGRESS = 0x01;
    private static final int MESSAGE_ERROR = 0x02;
    private static final int MESSAGE_DURATION = 0x03;
    private static final int MESSAGE_FORMAT = 0x04;

    private static final String WEB_HTTP_URL = "http://www.modrails.com/videos/passenger_nginx.mov";

    private Button mBtnDurationWeb;
    private Button mBtnDurationLocal;

    private Button mBtnFormatWeb;
    private Button mBtnFormatLocal;


    private WeakHandler mHandler = new WeakHandler(Looper.getMainLooper(), new Handler.Callback() {
        @Override
        public boolean handleMessage(Message msg) {
            switch(msg.what) {
                case MESSAGE_SHOW_PROGRESS:
                    showProgressDialog(getString(R.string.probing_video));
                    return true;
                case MESSAGE_DISMISS_PROGRESS:
                    dismissProgressDialog();
                    return true;
                case MESSAGE_ERROR:
                    Toast.makeText(getContext(), R.string.probe_error, Toast.LENGTH_LONG).show();
                    return true;
                case MESSAGE_DURATION:
                    VideoDuration.Format duration = (VideoDuration.Format)msg.obj;
                    Toast.makeText(getContext(), getString(R.string.probe_duration_finish, duration.mDuration),
                            Toast.LENGTH_LONG).show();
                    return true;
                case MESSAGE_FORMAT:
                    VideoFormat.Format format = (VideoFormat.Format)msg.obj;
                    Toast.makeText(getContext(), getString(R.string.probe_format_finish, format.mFormatName),
                            Toast.LENGTH_LONG).show();
                    return true;
            }
            return false;
        }
    });


    public MainActivityFragment() {
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        View layout = inflater.inflate(R.layout.fragment_main, container, false);
        mBtnDurationWeb = layout.findViewById(R.id.btn_duration_web);
        mBtnDurationLocal = layout.findViewById(R.id.btn_duration_local);
        mBtnFormatWeb = layout.findViewById(R.id.btn_format_web);
        mBtnFormatLocal = layout.findViewById(R.id.btn_format_local);
        return layout;
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mBtnDurationWeb.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(ProbeLogic.isProbeInit()) {
                    new ProbeDurationTask(mHandler).execute(WEB_HTTP_URL);
                }
            }
        });
        mBtnDurationLocal.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(ProbeLogic.isProbeInit()) {
                    new ProbeDurationTask(mHandler).execute(App.getInstance().getDestFile().getAbsolutePath());
                }
            }
        });
        mBtnFormatWeb.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(ProbeLogic.isProbeInit()) {
                    new ProbeFormatTask(mHandler).execute(WEB_HTTP_URL);
                }
            }
        });
        mBtnFormatLocal.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(ProbeLogic.isProbeInit()) {
                    new ProbeFormatTask(mHandler).execute(App.getInstance().getDestFile().getAbsolutePath());
                }
            }
        });
    }

    private static class ProbeDurationTask extends AsyncTask<String, Void, VideoDuration> {

        private WeakHandler mHandler;

        ProbeDurationTask(WeakHandler handler) {
            this.mHandler = handler;
        }

        @Override
        protected VideoDuration doInBackground(String... strings) {
            publishProgress();
            return ProbeLogic.probeVideoDuration(strings[0]);
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            mHandler.sendEmptyMessage(MESSAGE_SHOW_PROGRESS);
        }

        @Override
        protected void onPostExecute(final VideoDuration videoDuration) {
            mHandler.sendEmptyMessage(MESSAGE_DISMISS_PROGRESS);
            Message msg = mHandler.obtainMessage(MESSAGE_DURATION);
            if(videoDuration != null && videoDuration.mFormat != null) {
                msg.obj = videoDuration.mFormat;
                msg.sendToTarget();
            }
        }
    }

    private static class ProbeFormatTask extends AsyncTask<String, Void, VideoFormat> {

        private WeakHandler mHandler;

        ProbeFormatTask(WeakHandler handler) {
            this.mHandler = handler;
        }

        @Override
        protected VideoFormat doInBackground(String... strings) {
            publishProgress();
            return ProbeLogic.probeVideoFormat(strings[0]);
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            mHandler.sendEmptyMessage(MESSAGE_SHOW_PROGRESS);
        }

        @Override
        protected void onPostExecute(final VideoFormat videoFormat) {
            mHandler.sendEmptyMessage(MESSAGE_DISMISS_PROGRESS);
            Message msg = mHandler.obtainMessage(MESSAGE_FORMAT);
            if(videoFormat != null && videoFormat.mFormat != null) {
                msg.obj = videoFormat.mFormat;
                msg.sendToTarget();
            } else {
                mHandler.sendEmptyMessage(MESSAGE_ERROR);
            }
        }
    }

    private ProgressDialog mProgressDialog;

    private void showProgressDialog(String msg) {
        if(mProgressDialog == null)
            mProgressDialog = new ProgressDialog(getContext());
        mProgressDialog.setMessage(msg);
        if(!mProgressDialog.isShowing())
            mProgressDialog.show();
    }

    private void dismissProgressDialog() {
        if(mProgressDialog == null)
            return;
        if(mProgressDialog.isShowing())
            mProgressDialog.dismiss();
    }


}
