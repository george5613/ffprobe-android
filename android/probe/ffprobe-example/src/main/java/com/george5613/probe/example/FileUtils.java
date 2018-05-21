package com.george5613.probe.example;

import android.content.Context;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class FileUtils {

    private static final String TAG = FileUtils.class.getCanonicalName();

    private static final int DEFAULT_BUFFER_SIZE = 1024 * 4;
    private static final int EOF = -1;

    public static File getFilesDirectory(Context context) {
        // creates files directory under data/data/package name
        return context.getFilesDir();
    }

    public static boolean copyBinaryFromAssetsToData(Context context, String fileNameFromAssets,
            String outputFileName) {

        // create files directory under /data/data/package name
        File filesDirectory = getFilesDirectory(context);

        InputStream is;
        try {
            is = context.getAssets().open(fileNameFromAssets);
            // copy binary file from assets to files dir
            final FileOutputStream os = new FileOutputStream(new File(filesDirectory,
                    outputFileName));
            byte[] buffer = new byte[DEFAULT_BUFFER_SIZE];

            int n;
            while(EOF != (n = is.read(buffer))) {
                os.write(buffer, 0, n);
            }

            close(os);
            close(is);

            return true;
        } catch(IOException e) {
            Log.e(TAG, "issue in coping binary from assets to data. " + e.getMessage());
        }
        return false;
    }

    private static void close(InputStream inputStream) {
        if (inputStream != null) {
            try {
                inputStream.close();
            } catch (IOException e) {
                // Do nothing
            }
        }
    }

    private static void close(OutputStream outputStream) {
        if (outputStream != null) {
            try {
                outputStream.flush();
                outputStream.close();
            } catch (IOException e) {
                // Do nothing
            }
        }
    }

}
