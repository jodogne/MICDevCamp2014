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
import android.widget.ImageView;
import android.view.Display;
import android.graphics.Point;
import android.view.ViewGroup.LayoutParams;
import java.io.File;
import android.provider.MediaStore;
import android.net.Uri;
import android.widget.Spinner;

// Toast
import android.widget.Toast;
import android.view.View;
import android.view.View.OnClickListener;

public class Photo extends Activity 
{
	private String sessionId_ = "";
	private String siteId_ = "";
	private String siteName_ = "";

	private ImageView imageView_ = null;
	private Spinner tags_ = null;
	private Button captureButton_ = null;
	private Button uploadButton_ = null;


	private void ProcessIntent(Intent intent)
	{
		if (intent == null) {
			return;
		}

		if (intent.hasExtra(Shared.SESSION_ID)) {
			sessionId_ = intent.getStringExtra(Shared.SESSION_ID);
		}

		if (intent.hasExtra(Shared.SITE_ID)) {
			siteId_ = intent.getStringExtra(Shared.SITE_ID);
		}

		if (intent.hasExtra(Shared.SITE_NAME)) {
			siteName_ = intent.getStringExtra(Shared.SITE_NAME);
		}

		if (intent.hasExtra(RestToolbox.HTTP_REQUEST) &&
			intent.hasExtra(RestToolbox.HTTP_ANSWER))
		{
			String request = intent.getStringExtra(RestToolbox.HTTP_REQUEST);
			String answer = intent.getStringExtra(RestToolbox.HTTP_ANSWER);

			if (request.equals("send-photo") && answer != null) {
				try {
					JSONObject json = new JSONObject(answer);
					// Successful POST
					Toast.makeText(getApplicationContext(), "Image successfully uploaded!", Toast.LENGTH_LONG).show();
					finish();
				}
				catch (org.json.JSONException e) {
				}
			}
		}

	}


	@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.photo);
		ProcessIntent(getIntent());

		imageView_ = (ImageView) findViewById(R.id.imageView);
		tags_ = (Spinner) findViewById(R.id.photoTag);
		captureButton_ = (Button) findViewById(R.id.captureButton);
		uploadButton_ = (Button) findViewById(R.id.uploadButton);
		uploadButton_.setEnabled(false);

		captureButton_.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				TakePicture();
			}
		});

		uploadButton_.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				Upload();
			}
		});
    }


	@Override
	protected void onNewIntent(Intent intent) {
		ProcessIntent(intent);
	}


	@Override
	public void onStart(){
		super.onStart();

		((TextView) findViewById(R.id.siteName)).setText(siteName_);
		AdjustImageViewSize();
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



	private void AdjustImageViewSize() {
		// http://stackoverflow.com/a/1016941/881731
		Display display = getWindowManager().getDefaultDisplay();
		Point size = new Point();
		display.getSize(size);
		int width = size.x;
		int height = size.y;

		int s = Math.min(width, height) / 2;
		Log.d(Shared.LOGGER_TAG, "Screen size: " + width + "x" + height + " => resize " + s + "x" + s);

		LayoutParams params = imageView_.getLayoutParams();
		params.height = s;
		params.width = s;
		imageView_.setLayoutParams(params);
	}



	static final int REQUEST_TAKE_PHOTO = 1;

	private File GetTargetImageFile() {
		Log.d(Shared.LOGGER_TAG, "path " + getExternalFilesDir(null));
		return new File(getExternalFilesDir(null), "capture.jpg");
	}

	private void TakePicture() {
		Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
		if (takePictureIntent.resolveActivity(getPackageManager()) != null) {
			takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, Uri.fromFile(GetTargetImageFile()));
			startActivityForResult(takePictureIntent, REQUEST_TAKE_PHOTO);
			Log.d(Shared.LOGGER_TAG, "PHOTO IS BEING TAKEN to: " + GetTargetImageFile().getAbsolutePath());
		}
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		Log.d(Shared.LOGGER_TAG, "PHOTO TAKEN");

		if (requestCode == REQUEST_TAKE_PHOTO && 
			resultCode == RESULT_OK) {
			Shared.DisplayThumbnail(imageView_, GetTargetImageFile().getAbsolutePath());
			uploadButton_.setEnabled(true);
		}
	}

	private void Upload() {
		try {
			JSONObject obj = new JSONObject();
			obj.put("Tag", tags_.getSelectedItem().toString());
			obj.put("SecondsSinceEpoch", String.valueOf(Shared.GetSecondsSinceEpoch()));
			obj.put("ImageMime", "image/jpeg");
			obj.put("SiteUuid", siteId_);
			Log.d(Shared.LOGGER_TAG, "Sending photo (size " + 
				  String.valueOf(GetTargetImageFile().length()) + ") : " + obj.toString());
			obj.put("ImageData", Shared.ComputeBase64(GetTargetImageFile()));
			RestToolbox.DoHttpPost(this, "send-photo", PhotoTrackApi.SERVER_URL + "photos", obj.toString(), sessionId_);
		}
		catch (org.json.JSONException e) {
		}
		catch (java.io.IOException e) {
		}
	}

}
