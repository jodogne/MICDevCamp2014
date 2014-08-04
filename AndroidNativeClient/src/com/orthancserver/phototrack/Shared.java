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
import java.io.File;
import android.util.Base64;
import java.io.FileInputStream;

// Toast
import android.widget.Toast;
import android.view.View;
import android.view.View.OnClickListener;

// Various for image manipulation
import android.widget.ImageView;
import android.media.ExifInterface;
import android.graphics.BitmapFactory;
import android.graphics.Point;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.view.Display;
import android.view.ViewGroup.LayoutParams;
import android.graphics.Bitmap;

public class Shared extends Activity 
{
	public static final String LOGGER_TAG = "OrthancPhotoTrack";
	public final static String SESSION_ID = "com.orthancserver.phototrack.SESSION_ID";
	public final static String SITE_ID = "com.orthancserver.phototrack.SITE_ID";
	public final static String SITE_NAME = "com.orthancserver.phototrack.SITE_NAME";

	private static final int MENU_LOGOUT = 10;

	static public void CreateMenu(Activity activity, 
								  Menu menu)
	{
		// setIcon(): http://androiddrawableexplorer.appspot.com/
		// Pour Android 2.1: http://docs.since2006.com/android/2.1-drawables.php


		// /home/jodogne/Downloads/Android/adt-bundle-linux-x86_64/sdk/platforms/android-7/data/res/drawable-hdpi
		//   => Look at what begins with "ic_menu_*"

		MenuItem menuItem;

		menuItem = menu.add(Menu.NONE, MENU_LOGOUT, Menu.NONE, 
							activity.getString(R.string.menuLogout));
		menuItem.setIcon(android.R.drawable.ic_menu_close_clear_cancel);
	}


	static public void OnMenuItemSelected(MenuItem item,
										  Activity activity,
										  String sessionId)
	{
		switch (item.getItemId()) 
		{
			case MENU_LOGOUT:
				PhotoTrackApi.Logout(activity, sessionId);
				break;

			default:
				Log.e(LOGGER_TAG, "Unknown menu item");	
		}
	}



	static public void DisplayThumbnail(ImageView imageView,
										String path) 
	{
		File file = new File(path);

		if (!file.exists())
		{
			imageView.setImageDrawable(null);  // Clear image
			return;
		}

		// Get the dimensions of the View
		int targetW = imageView.getWidth();
		int targetH = imageView.getHeight();
		if (targetW == 0 || targetH == 0)
		{
			imageView.setImageDrawable(null);  // Clear image
			return;
		}

		// Get the dimensions of the bitmap
		BitmapFactory.Options bmOptions = new BitmapFactory.Options();
		bmOptions.inJustDecodeBounds = true;
		BitmapFactory.decodeFile(path, bmOptions);
		int photoW = bmOptions.outWidth;
		int photoH = bmOptions.outHeight;

		Log.d(LOGGER_TAG, "Current photo path: " + path);
		Log.d(LOGGER_TAG, "Image size: " + photoW + "x" + photoH + ", of " + 
			  new File(path).length() + " bytes");

		// Determine how much to scale down the image
		int scaleFactor = Math.min(photoW/targetW, photoH/targetH);

		// Decode the image file into a Bitmap sized to fill the View
		bmOptions.inJustDecodeBounds = false;
		bmOptions.inSampleSize = scaleFactor;
		bmOptions.inPurgeable = true;

		Bitmap bitmap = BitmapFactory.decodeFile(path, bmOptions);

		// Correct orientation according to EXIF information
		// http://stackoverflow.com/questions/6813166/set-orientation-of-android-camera-started-with-intent-action-image-capture

		try {
			ExifInterface exif = new ExifInterface(path);	
			int orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_NORMAL);
			int rotate = 0;

			// http://stackoverflow.com/a/12790765/881731
			if (orientation == ExifInterface.ORIENTATION_ROTATE_90) {
				rotate = 90;
			} else if (orientation == ExifInterface.ORIENTATION_ROTATE_180) {
				rotate = 180;
			} else if (orientation == ExifInterface.ORIENTATION_ROTATE_270) {
				rotate = 270;
			}

			Log.d(LOGGER_TAG, "Correcting EXIF orientation: " + rotate);
			
			if (rotate != 0) {
				Matrix matrix = new Matrix();
				matrix.postRotate(rotate);
				bitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
			}
		} catch (java.io.IOException e) {
			Log.d(LOGGER_TAG, "Unable to read the EXIF information!");
		}

		imageView.setImageBitmap(bitmap);
	}

	static long GetSecondsSinceEpoch()
	{
		return System.currentTimeMillis() / 1000;
	}


	static String ComputeBase64(File file) throws java.io.IOException
	{
        byte[] content = new byte[(int) file.length()];
 
		//convert file into array of bytes
		FileInputStream fileInputStream = new FileInputStream(file);
		fileInputStream.read(content);
		fileInputStream.close();

		return Base64.encodeToString(content, Base64.NO_WRAP);
	}
}
