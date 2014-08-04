/**
 * Orthanc PhotoTrack
 * Copyright (C) 2014 - Gregory Art, Jean-Francois Colson, Benjamin
 * Golinvaux, Sebastien Jodogne
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

package com.orthancserver.phototrack;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;
import android.widget.Button;
import android.widget.EditText;
import org.json.JSONObject;

// Toast
import android.widget.Toast;
import android.view.View;
import android.view.View.OnClickListener;

public class Login extends Activity 
{
	private TextView serverUrl_ = null;
	private Button button_ = null;
	private EditText username_ = null;
	private EditText password_ = null;

	@Override
	public void onBackPressed() {
		// Disable back button
	}


	@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.login);

		button_ = (Button) findViewById(R.id.button);
		username_ = (EditText) findViewById(R.id.username);
		password_ = (EditText) findViewById(R.id.password);

		// TODO
		username_.setText("user");
		password_.setText("pass");
		
		serverUrl_ = (TextView) findViewById(R.id.serverUrl);
		serverUrl_.setText(PhotoTrackApi.SERVER_URL);
		
		final Activity activity = this;
		button_.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				PhotoTrackApi.Login(activity, 
									username_.getText().toString(), 
									password_.getText().toString());
			}
		});
    }


	@Override
	protected void onNewIntent(Intent intent) {
		Log.d(Shared.LOGGER_TAG, "RECEIVED INTENT");
		if (intent != null) {
			String request = intent.getStringExtra(RestToolbox.HTTP_REQUEST);
			String answer = intent.getStringExtra(RestToolbox.HTTP_ANSWER);
			Log.d(Shared.LOGGER_TAG, "REQUEST: " + request + ", RESPONSE: " + answer);

			if (request.equals("login")) {
				if (answer == null) {
				  Toast.makeText(getApplicationContext(), "Bad login", Toast.LENGTH_LONG).show();
				} else {
					try {
						JSONObject json = new JSONObject(answer);
						Intent intent2 = new Intent(this, ListSites.class);
						intent2.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
						intent2.putExtra(Shared.SESSION_ID, json.getString("SessionId"));
						startActivity(intent2);
					}
					catch (org.json.JSONException e) {
						Toast.makeText(getApplicationContext(), "Bad REST answer", Toast.LENGTH_LONG).show();
					}
				}
			}
		}
	}


	@Override
	public void onStart(){
		super.onStart();
	}
}
