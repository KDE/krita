 /*
  * This file is part of the KDE project
  * Copyright (C) 2019 Sharaf Zaman <sharafzaz121@gmail.com>
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 2 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
  */

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
