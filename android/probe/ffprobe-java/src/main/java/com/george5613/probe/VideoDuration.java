package com.george5613.probe;

import android.os.Parcel;
import android.os.Parcelable;

import com.alibaba.fastjson.annotation.JSONField;
import com.alibaba.fastjson.annotation.JSONType;

/**
 * Created by George on 16/5/11.
 */
@JSONType
public class VideoDuration {

    @JSONField(name = "format")
    public Format mFormat;

    @JSONType
    public static class Format implements Parcelable {

        @JSONField(name = "duration")
        public double mDuration;
        @JSONField(name = "start_time")
        public double mStartTime;

        public Format() {
        }

        Format(Parcel in) {
            mDuration = in.readDouble();
            mStartTime = in.readDouble();
        }

        public static final Creator<Format> CREATOR = new Creator<Format>() {
            @Override
            public Format createFromParcel(Parcel in) {
                return new Format(in);
            }

            @Override
            public Format[] newArray(int size) {
                return new Format[size];
            }
        };

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeDouble(mDuration);
            dest.writeDouble(mStartTime);
        }
    }

}
