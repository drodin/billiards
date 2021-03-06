package com.drodin.billiards;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.widget.LinearLayout;

public class Installer extends Activity {
	@Override
	protected void onCreate(Bundle icicle) {
		super.onCreate(icicle);

		getWindow().setFlags(
				WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
				WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		LinearLayout mView = new LinearLayout(getApplicationContext());
		mView.setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
		mView.setBackground(getDrawable(R.drawable.splash));

		mView.setSystemUiVisibility(
				View.SYSTEM_UI_FLAG_IMMERSIVE
						// Set the content to appear under the system bars so that the
						// content doesn't resize when the system bars hide and show.
						| View.SYSTEM_UI_FLAG_LAYOUT_STABLE
						| View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
						| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
						// Hide the nav bar and status bar
						| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
						| View.SYSTEM_UI_FLAG_FULLSCREEN);

		setContentView(mView,
				new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));

		new Thread(new Runnable() {
			public void run() {
				InstallFiles();
				startActivity(new Intent(getApplicationContext(), MySDLActivity.class));
				finish();
			}
		}).start();

	}
	
	public void InstallFiles() {
		final AssetManager mAssetManager = getApplication().getResources().getAssets();

		try {
			String DATA_DIR = getApplicationContext().getFilesDir().getAbsolutePath();

			copyAssetFolder(mAssetManager, "data", DATA_DIR);
		}
		catch(Exception e) {
			e.printStackTrace();
		}
	}

	private static boolean copyAssetFolder(AssetManager assetManager, String fromAssetPath, String toPath) {
		try {
			String[] files = assetManager.list(fromAssetPath);
			if (files.length == 0)
				copyAsset(assetManager, fromAssetPath, toPath);
			else {
				new File(toPath).mkdirs();
				for (String file : files)
					copyAssetFolder(assetManager, fromAssetPath + "/" + file, toPath + "/" + file);
			}
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		return true;
	}

	private static boolean copyAsset(AssetManager assetManager, String fromAssetPath, String toPath) {
		InputStream in;
		OutputStream out;

		try {
			in = assetManager.open(fromAssetPath);
			File toFile = new File(toPath);

			if (toFile.exists() && toFile.length() == in.available()) //not for big files
				return true;

			out = new FileOutputStream(toFile);
			copyFile(in, out);
			in.close();
			out.flush();
			out.close();
			return true;
		} catch(Exception e) {
			e.printStackTrace();
			return false;
		}
	}

	private static void copyFile(InputStream in, OutputStream out) throws IOException {
		byte[] buffer = new byte[1024];
		int read;
		while((read = in.read(buffer)) != -1){
			out.write(buffer, 0, read);
		}
	}
}
