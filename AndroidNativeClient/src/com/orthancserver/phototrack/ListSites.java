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
import android.widget.AdapterView;
import android.widget.ListView;
import org.json.JSONObject;
import org.json.JSONArray;
import android.widget.ArrayAdapter;  
import java.util.ArrayList;  
import android.widget.AdapterView.OnItemClickListener;

// Toast
import android.widget.Toast;
import android.view.View;
import android.view.View.OnClickListener;

public class ListSites extends Activity 
{
	private static final int MENU_LOGOUT = 10;

	private String sessionId_ = "";
    private ArrayList<String> sites_ = new ArrayList<String>();
    private ArrayList<String> sitesUuids_ = new ArrayList<String>();
	private ListView listView_;
	private ArrayAdapter adapter_;

	@Override
	public void onBackPressed() {
		// Disable back button to avoid returning to the login screen

		// Go to home screen instead of previous Activity
		// http://stackoverflow.com/a/11807648/881731
		/*Intent startMain = new Intent(Intent.ACTION_MAIN);
        startMain.addCategory(Intent.CATEGORY_HOME);
        startMain.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(startMain);*/
	}

	private void ProcessIntent(Intent intent)
	{
		if (intent == null) {
			return;
		}

		if (intent.hasExtra(RestToolbox.HTTP_REQUEST) &&
			intent.hasExtra(RestToolbox.HTTP_ANSWER))
		{
			String request = intent.getStringExtra(RestToolbox.HTTP_REQUEST);
			String answer = intent.getStringExtra(RestToolbox.HTTP_ANSWER);

			if (request.equals("sites") && answer != null) {
				sites_.clear();
				sitesUuids_.clear();
				adapter_.notifyDataSetChanged();

				try {
					JSONArray json = new JSONArray(answer);
					for (int i = 0; i < json.length(); i++) {
						JSONObject site = json.getJSONObject(i);

						String s = "";
						if (site.has("Name")) {
							s += site.getString("Name");
						}

						if (site.has("PitNumber")) {
							if (s.length() > 0)
								s += " - ";
							s += site.getString("PitNumber") + " ";
						}

						sites_.add(s);
						sitesUuids_.add(site.getString("Uuid"));

						adapter_.notifyDataSetChanged();
					}
				}
				catch (org.json.JSONException e) {
				}
			}
		}

		if (intent.hasExtra(Shared.SESSION_ID)) {
			sessionId_ = intent.getStringExtra(Shared.SESSION_ID);
			Log.d(Shared.LOGGER_TAG, "Session ID in ListSites: " + sessionId_);
		}
	}



	@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.list_sites);
		ProcessIntent(getIntent());

		listView_ = (ListView) findViewById(R.id.listView);
		adapter_ = new ArrayAdapter(this, android.R.layout.simple_list_item_1, sites_);
		listView_.setAdapter(adapter_);

		final Activity that = this;
		listView_.setOnItemClickListener(new OnItemClickListener()
		{
			@Override 
			public void onItemClick(AdapterView<?> arg0, View arg1, int position, long arg3)
			{ 
				if (position < sitesUuids_.size()) {
					String name = sites_.get(position);
					String uuid = sitesUuids_.get(position);

					Intent intent = new Intent(that, Photo.class);
					intent.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
					intent.putExtra(Shared.SESSION_ID, sessionId_);
					intent.putExtra(Shared.SITE_NAME, name);
					intent.putExtra(Shared.SITE_ID, uuid);
					startActivity(intent);
				}
			}
		});
    }


	@Override
	protected void onNewIntent(Intent intent) {
		ProcessIntent(intent);
	}


	@Override
	public void onStart() {
		super.onStart();
		RestToolbox.DoHttpGet(this, "sites", PhotoTrackApi.SERVER_URL + "sites", sessionId_);
	}


	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		super.onCreateOptionsMenu(menu);
		Shared.CreateMenu(this, menu);
		return true;
	}


	@Override
	public boolean onOptionsItemSelected(MenuItem item) 
	{
		super.onOptionsItemSelected(item);
		Shared.OnMenuItemSelected(item, this, sessionId_);
		return false;
	}

}
