package com.example.the5bug.the5bugs;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.animation.AlphaAnimation;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import io.particle.android.sdk.cloud.ParticleCloud;
import io.particle.android.sdk.cloud.ParticleCloudException;
import io.particle.android.sdk.cloud.ParticleCloudSDK;
import io.particle.android.sdk.cloud.ParticleDevice;
import io.particle.android.sdk.utils.Async;
import io.particle.android.sdk.utils.Toaster;

public class MainActivity extends AppCompatActivity {

    protected Button armButton;
    protected Button disButton;
    protected Button changePhone;
    protected TextView connectText;
    private AlphaAnimation buttonClick = new AlphaAnimation(1F, 0.8F);
    String disarmString = "Detector now disarmed";
    String armString = "Detector now armed";
    ParticleDevice d;
    Button b1, b2;
    EditText ed1, ed2;

    TextView tx1;
    int counter = 3;
    ParticleDevice ourDevice;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ourDevice = getIntent().getParcelableExtra("ourDevice");

        connectText = (TextView)findViewById(R.id.textView5);

        connectText.setText(ourDevice.getName());

        final View viewF = findViewById(android.R.id.content);




        armButton = (Button)findViewById(R.id.Armbutton);
        disButton = (Button)findViewById(R.id.disarmButton);
        changePhone = (Button)findViewById(R.id.phone);


        armButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                //armButton.setClickable(false);
                armButton.setBackgroundColor(Color.GRAY);
                disButton.setBackgroundColor(Color.LTGRAY);

                Async.executeAsync(ParticleCloudSDK.getCloud(), new Async.ApiWork<ParticleCloud, Integer>() {
                    @Override
                    public Integer callApi(ParticleCloud particleCloud) throws ParticleCloudException, IOException {
                        List<String> list = new ArrayList<>();
                        list.add("Arm");
                        try{
                            return ourDevice.callFunction("setMotion", list);
                        }catch(ParticleDevice.FunctionDoesNotExistException e){
                            Log.e("ERR", e.toString());
                        }
                        return -2;
                    }

                    public void onSuccess(Integer result) {
                        Toast.makeText(getApplicationContext(),"You did it.", Toast.LENGTH_SHORT).show();
                    }
                    public void onFailure(ParticleCloudException value) {
                        Log.e("ERR", "Fail : " + value.toString());
                    }
                });
            }
        });

        disButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View view2) {

                //disButton.setClickable(false);
                //armButton.setClickable(true);
                disButton.setBackgroundColor(Color.GRAY);
                armButton.setBackgroundColor(Color.LTGRAY);


                Async.executeAsync(ParticleCloudSDK.getCloud(), new Async.ApiWork<ParticleCloud, Integer>() {
                    @Override
                    public Integer callApi(ParticleCloud particleCloud) throws ParticleCloudException, IOException {
                        List<String> list = new ArrayList<>();
                        list.add("Disarm");
                        try{
                            return ourDevice.callFunction("setMotion", list);
                        }catch(ParticleDevice.FunctionDoesNotExistException e){
                            Log.e("ERR", e.toString());
                        }
                        return -2;
                    }

                    public void onSuccess(Integer result) {
                        Toast.makeText(getApplicationContext(),"You did it.", Toast.LENGTH_SHORT).show();
                    }
                    public void onFailure(ParticleCloudException value) {
                        Log.e("ERR", "Fail : " + value.toString());
                    }
                });
            }
        });

        changePhone.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                startActivity(new Intent(MainActivity.this, Settings.class));

            }
        });


    }

//    private void turnOn(View myView){
//
//        Async.executeAsync(ParticleCloud.get(myView.getContext()), new Async.ApiWork<ParticleCloud, Void>() {
//
//            public Void callApi(ParticleCloud sparkCloud) throws ParticleCloudException, IOException {
//                try{
//                    ourDevice.callFunction("Arm", null);
//                    Toast.makeText(getApplicationContext(), "Armed.", Toast.LENGTH_SHORT).show();
//                }catch(Exception e){
//                    Toast.makeText(getApplicationContext(),"ERROR",Toast.LENGTH_SHORT).show();
//                }
//
//                return null;
//            }
//
//            @Override
//            public void onSuccess(Void aVoid) {
//                Toaster.l(MainActivity.this, "System is armed.");
//            }
//
//            @Override
//            public void onFailure(ParticleCloudException e) {
//                //Log.e("LOGIN_FAILURE", e);
//                Toaster.l(MainActivity.this, "ERROR RUNNING CODE.");
//            }
//        });
//    }

//    private void turnOff(View myView){
//
//        Async.executeAsync(ParticleCloud.get(myView.getContext()), new Async.ApiWork<ParticleCloud, Void>() {
//
//            public Void callApi(ParticleCloud sparkCloud) throws ParticleCloudException, IOException {
//                try{
//                    ourDevice.callFunction("Disarm");
//                    Toast.makeText(getApplicationContext(), "Disarmed.", Toast.LENGTH_SHORT).show();
//                }catch(Exception e){
//                    Toast.makeText(getApplicationContext(),"ERROR",Toast.LENGTH_SHORT).show();
//            }
//
//                return null;
//            }
//
//            @Override
//            public void onSuccess(Void aVoid) {
//                Toaster.l(MainActivity.this, "System is armed.");
//            }
//
//            @Override
//            public void onFailure(ParticleCloudException e) {
//                //Log.e("LOGIN_FAILURE", e);
//                Toaster.l(MainActivity.this, "ERROR RUNNING CODE.");
//            }
//        });
//    }


}
