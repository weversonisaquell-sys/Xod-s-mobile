package com.gabedeveloper.xodo;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.ScrollView;

public class MainActivity extends Activity {

    // Carrega a biblioteca nativa compilada a partir do cpp/
    static {
        System.loadLibrary("xodocore");
    }

    // Declaracao do metodo nativo implementado em native-lib.cpp
    public native String runDemo();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        TextView output = new TextView(this);
        output.setPadding(32, 32, 32, 32);
        output.setTextSize(14);

        ScrollView scroll = new ScrollView(this);
        scroll.addView(output);
        setContentView(scroll);

        String resultado = runDemo();
        output.setText("XODO EMULATOR - nucleo 8086\n\n" + resultado);
    }
}

