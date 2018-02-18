package com.example.the5bug.the5bugs;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import java.io.IOException;

import io.particle.android.sdk.cloud.ParticleCloud;
import io.particle.android.sdk.cloud.ParticleCloudException;
import io.particle.android.sdk.cloud.ParticleDevice;
import io.particle.android.sdk.utils.Async;
import io.particle.android.sdk.utils.Toaster;

public class Login extends AppCompatActivity {

    Button loginBtn;
    EditText usr, psd;
    ParticleDevice ourDevice;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        usr = (EditText) findViewById(R.id.usr);
        psd = (EditText) findViewById(R.id.psd);
        loginBtn = (Button)findViewById(R.id.loginBtn);

        loginBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                View viewd = findViewById(android.R.id.content);

                attemptLogin(viewd, usr.getText().toString(),psd.getText().toString());
            }
        });


    }

    /**
     * @brief This function attempts to login with the user name and password passed in as arguments
     * @param myView specifies the view from which this is being called
     * @param emailInput specifies the user name to try logging in with
     * @param passInput specifies the password to attempt logging in with
     */
    private void attemptLogin(View myView, final String emailInput, final String passInput){

        Async.executeAsync(ParticleCloud.get(myView.getContext()), new Async.ApiWork<ParticleCloud, Void>() {

            public Void callApi(ParticleCloud sparkCloud) throws ParticleCloudException, IOException {
                sparkCloud.logIn(emailInput, passInput);
                ourDevice = sparkCloud.getDevice("3b0029000f51353433323633");
                return null;
            }

            @Override
            public void onSuccess(Void aVoid) {

                Toaster.l(Login.this, "Logged in");

                Intent myintent = new Intent(getBaseContext(), MainActivity.class);
                myintent.putExtra("ourDevice", ourDevice);
                startActivity(myintent);
                // Switch to new screen to list all connected devices

            }

            @Override
            public void onFailure(ParticleCloudException e) {
                //Log.e("LOGIN_FAILURE", e);
                Toaster.l(Login.this, "Wrong credentials or no internet connectivity, please try again");
            }
        });
    }
}
