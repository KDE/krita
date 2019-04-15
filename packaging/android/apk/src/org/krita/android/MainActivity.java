package org.krita.android;

import android.os.Bundle;
import android.view.WindowManager;

import org.qtproject.qt5.android.bindings.QtActivity;

public class MainActivity extends QtActivity {

	@Override
	public void onCreate(Bundle savedInstanceState) {
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
		           WindowManager.LayoutParams.FLAG_FULLSCREEN);
		super.onCreate(savedInstanceState);

		new ConfigsManager().handleAssets(this);
	}

}
