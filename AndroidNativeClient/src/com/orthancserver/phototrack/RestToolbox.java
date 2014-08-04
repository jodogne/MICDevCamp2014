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
import android.os.AsyncTask;
import android.content.Intent;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import android.util.Log;

public class RestToolbox
{
	public final static String HTTP_REQUEST = "com.orthancserver.phototrack.HTTP_REQUEST";
	public final static String HTTP_ANSWER = "com.orthancserver.phototrack.HTTP_ANSWER";

	static private String ReadStream(InputStream stream) {
		try {
			BufferedReader reader = new BufferedReader(new InputStreamReader(stream, "UTF-8"), 8);
			StringBuilder sb = new StringBuilder();

			String line = null;
			while ((line = reader.readLine()) != null)
			{
				sb.append(line + "\n");
			}

			return sb.toString();
		}
		catch (java.io.UnsupportedEncodingException e) {
			return null;
		}
		catch (java.io.IOException e) {
			return null;
		}
	}

	static private void WriteStream(OutputStream stream, 
									String str)
	{
		try {
			DataOutputStream wr = new DataOutputStream(stream);
			wr.writeBytes(str);
			wr.flush();
			wr.close();
		}
		catch (java.io.IOException e) {
		}
	}

	private enum HttpMethod {
		GET, POST, DELETE
			}

	static private class HttpRequestParams {
		Activity activity_;
		Class<?> callee_;
		String request_;
		String url_;
		String body_ = null;
		HttpMethod method_ = HttpMethod.GET;
		String sessionId_ = null;
	}

	static private class HttpRequestResult {
		Activity activity_;
		Class<?> callee_;
		String request_;
		String answer_;
	}


	static private class DoHttpTask extends AsyncTask<
									HttpRequestParams /*inputs*/,
									String /*progress*/, 
									HttpRequestResult /* output */> {
		@Override
			protected HttpRequestResult doInBackground(HttpRequestParams... params) {

			HttpRequestResult result = new HttpRequestResult();
			result.activity_ = params[0].activity_;
			result.request_ = params[0].request_;
			result.callee_ = params[0].callee_;

			try {
				URL url = new URL(params[0].url_);
				HttpURLConnection connection = (HttpURLConnection) url.openConnection();

				if (params[0].sessionId_ != null)
				{
					connection.addRequestProperty("Cookie", "session=" + params[0].sessionId_);
				}

				connection.setUseCaches(false);

				switch (params[0].method_) {
					case GET:
						break;

					case DELETE:
						connection.setRequestMethod("DELETE");
						break;

					case POST:
						connection.setRequestMethod("POST");
						connection.setRequestProperty("Content-Length", "" + 
													  Integer.toString(params[0].body_.getBytes().length));
						connection.setDoInput(true);
						connection.setDoOutput(true);
						WriteStream(connection.getOutputStream(), params[0].body_);
						break;
				}

				String answer = ReadStream(connection.getInputStream());
				connection.disconnect();

				result.answer_ = answer;
			}
			catch (java.net.MalformedURLException e) {
				result.answer_ = null;
			} catch (java.net.ProtocolException e) {
				result.answer_ = null;
			} catch (java.io.IOException e) {
				result.answer_ = null;
			}

			return result;
		}

		@Override
			protected void onPostExecute(HttpRequestResult result) {
			super.onPostExecute(result);

			Intent intent = new Intent(result.activity_.getApplicationContext(), 
									   result.callee_);
			intent.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
			intent.putExtra(HTTP_REQUEST, result.request_);
			intent.putExtra(HTTP_ANSWER, result.answer_);
			result.activity_.startActivity(intent);
		}
	}


	static public void DoHttpGet(Activity activity,
								 Class<?> callee,
								 String request,
								 String url,
								 String sessionId)
	{
		HttpRequestParams params = new HttpRequestParams();
		params.request_ = request;
		params.activity_ = activity;
		params.callee_ = callee;
		params.url_ = url;
		params.sessionId_ = sessionId;

		DoHttpTask task = new DoHttpTask();
		task.execute(params);
	}


	static public void DoHttpPost(Activity activity,
								  Class<?> callee,
								  String request,
								  String url,
								  String body,
								  String sessionId)
	{
		HttpRequestParams params = new HttpRequestParams();
		params.request_ = request;
		params.activity_ = activity;
		params.callee_ = callee;
		params.url_ = url;
		params.method_ = HttpMethod.POST;
		params.body_ = body;
		params.sessionId_ = sessionId;

		DoHttpTask task = new DoHttpTask();
		task.execute(params);
	}


	static public void DoHttpDelete(Activity activity,
									Class<?> callee,
									String request,
									String url,
									String sessionId)
	{
		HttpRequestParams params = new HttpRequestParams();
		params.request_ = request;
		params.activity_ = activity;
		params.callee_ = callee;
		params.url_ = url;
		params.method_ = HttpMethod.DELETE;
		params.sessionId_ = sessionId;

		DoHttpTask task = new DoHttpTask();
		task.execute(params);

		String s = new String();
	}


	static public void DoHttpGet(Activity activity,
								 String request,
								 String url,
								 String sessionId)
	{
		DoHttpGet(activity, activity.getClass(), request, url, sessionId);
	}


	static public void DoHttpPost(Activity activity,
								  String request,
								  String url,
								  String body,
								  String sessionId)
	{
		DoHttpPost(activity, activity.getClass(), request, url, body, sessionId);
	}


	static public void DoHttpDelete(Activity activity,
									String request,
									String url,
									String sessionId)
	{
		DoHttpDelete(activity, activity.getClass(), request, url, sessionId);
	}
}
