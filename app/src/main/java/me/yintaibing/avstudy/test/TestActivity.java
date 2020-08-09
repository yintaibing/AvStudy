package me.yintaibing.avstudy.test;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import me.yintaibing.avstudy.R;

public class TestActivity extends AppCompatActivity {
    private Button button;
    private JniTest jniTest;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test);
        setTitle("Test");
        button = findViewById(R.id.btn_start);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String inputStr = String.valueOf(System.currentTimeMillis());
                String s = jniTest.nativeStringFromJNI(inputStr);
                button.setText(s);
            }
        });
        jniTest = new JniTest();
    }
}