package com.example.aaudiodemo;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import com.example.aaudiodemo.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'aaudiodemo' library on application startup.
    static {
        System.loadLibrary("aaudiodemo");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        binding.playAudio.setOnClickListener((v) -> {
        });
        binding.pauseAudio.setOnClickListener((v) -> {
        });
        binding.stopAudio.setOnClickListener((v) -> {
        });
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    public native long nativeCreateAAudioEngine(String filePath);

    public native void nativeDestroyAAudioEngine(long engineHandle);

    public native void nativeAAudioEnginePlay(long engineHandle);

    public native void nativeAAudioEnginePause(long engineHandle);

    public native void nativeAAudioEngineStop(long engineHandle);
}