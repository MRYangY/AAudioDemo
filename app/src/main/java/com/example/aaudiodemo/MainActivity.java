package com.example.aaudiodemo;

import androidx.appcompat.app.AppCompatActivity;

import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

import com.example.aaudiodemo.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private static final int INVALID_PTR = 0;

    // Used to load the 'aaudiodemo' library on application startup.
    static {
        System.loadLibrary("aaudiodemo");
    }

    private ActivityMainBinding binding;
    private long mEngineHandle = INVALID_PTR;
    private static final String TEST_FILE_PATH = "16k16bit.pcm";
    private static final int AUDIO_SAMPLERATE = 16000;
    private static final int AUDIO_CHANNELS = 1;
    private static final int AUDIO_FORMAT = 2;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        mEngineHandle = nativeCreateAAudioEngine(getAssets(), TEST_FILE_PATH, AUDIO_SAMPLERATE, AUDIO_CHANNELS, AUDIO_FORMAT);
        binding.initAudio.setOnClickListener(v -> {
            if (mEngineHandle == INVALID_PTR) {
                mEngineHandle = nativeCreateAAudioEngine(getAssets(), TEST_FILE_PATH, AUDIO_SAMPLERATE, AUDIO_CHANNELS, AUDIO_FORMAT);
            }
        });
        binding.playAudio.setOnClickListener((v) -> {
            if (mEngineHandle != INVALID_PTR) {
                nativeAAudioEnginePlay(mEngineHandle);
            }
        });
        binding.pauseAudio.setOnClickListener((v) -> {
            if (mEngineHandle != INVALID_PTR) {
                nativeAAudioEnginePause(mEngineHandle);
            }
        });
        binding.stopAudio.setOnClickListener((v) -> {
            if (mEngineHandle != INVALID_PTR) {
                nativeAAudioEngineStop(mEngineHandle);
                nativeDestroyAAudioEngine(mEngineHandle);
                mEngineHandle = INVALID_PTR;
            }
        });
    }

    public native long nativeCreateAAudioEngine(AssetManager assetMgr, String filePath, int sampleRate, int audioChannel, int audioFormat);

    public native void nativeDestroyAAudioEngine(long engineHandle);

    public native void nativeAAudioEnginePlay(long engineHandle);

    public native void nativeAAudioEnginePause(long engineHandle);

    public native void nativeAAudioEngineStop(long engineHandle);
}