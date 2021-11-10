package com.drodin.billiards;

import android.os.Bundle;

import org.libsdl.app.SDLActivity;

public class MySDLActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] {
                "billiards"
        };
    }
}
