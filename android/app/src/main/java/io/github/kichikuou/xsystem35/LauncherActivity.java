/* Copyright (C) 2019 <KichikuouChrome@gmail.com>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/
package io.github.kichikuou.xsystem35;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class LauncherActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent i = new Intent(Intent.ACTION_GET_CONTENT);
        i.setType("application/zip");
        startActivityForResult(Intent.createChooser(i, "Choose a file"), 0);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == RESULT_OK)
            extractFiles(data.getData(), getApplicationContext().getFilesDir());
        startGame();
    }

    boolean extractFiles(Uri zipUri, File outDir) {
        try {
            InputStream input = getContentResolver().openInputStream(zipUri);
            if (input == null)
                return false;
            ZipInputStream zip;
            if (Build.VERSION.SDK_INT >= 24)
                zip = new ZipInputStream(new BufferedInputStream(input), Charset.forName("Shift_JIS"));
            else
                zip = new ZipInputStream(new BufferedInputStream(input));
            byte[] buffer = new byte[4096];
            ZipEntry zipEntry;
            while ((zipEntry = zip.getNextEntry()) != null) {
                File entryName = new File(zipEntry.getName());
                if (zipEntry.isDirectory())
                    continue;
                Log.d("Extracting", entryName.toString());
                File outFile = new File(outDir, entryName.getName());
                BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(outFile));
                int size;
                while ((size = zip.read(buffer)) > 0) {
                    output.write(buffer, 0, size);
                }
                output.close();
            }
            zip.close();
            return true;
        } catch (IOException e) {
            Log.e("launcher", "Error while extracting zip", e);
            return false;
        }
    }

    void startGame() {
        Intent i = new Intent();
        i.setClass(getApplicationContext(), GameActivity.class);
        startActivity(i);
    }
}
