package org.krita.android;

import android.os.Bundle;
import android.view.WindowManager;

import org.qtproject.qt5.android.bindings.QtActivity;

public class MainActivity extends QtActivity {

	public boolean isStartup = true;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
		                     WindowManager.LayoutParams.FLAG_FULLSCREEN);
		super.onCreate(savedInstanceState);

		new ConfigsManager().handleAssets(this);
	}

	@Override
	public void onPause() {
		super.onPause();
		// onPause() _is_ called when the app starts. If the native lib
		// isn't loaded, it crashes.
		if (!isStartup) {
			JNIWrappers.saveState();
		}
		else {
			isStartup = false;
		}
	}
}
