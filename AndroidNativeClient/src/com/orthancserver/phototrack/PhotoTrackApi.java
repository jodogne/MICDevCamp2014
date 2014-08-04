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
import org.json.JSONObject;


public class PhotoTrackApi
{
	public static final String SERVER_URL = "http://192.169.1.106:8001/";

	static public void Login(Activity activity,
							 String username,
							 String password)
	{
		try {
			JSONObject json = new JSONObject();
			json.put("username", username);
			json.put("password", password);
			RestToolbox.DoHttpPost(activity, "login", SERVER_URL + "sessions", json.toString(), null);
		} 
		catch (org.json.JSONException e) {
		}
	}


	static public void Logout(Activity activity, 
							  String sessionId)
	{
		RestToolbox.DoHttpDelete(activity, Login.class, "logout", 
								 SERVER_URL + "sessions/" + sessionId, sessionId);
	}
}
