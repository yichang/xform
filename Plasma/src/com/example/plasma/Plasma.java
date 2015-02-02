/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.example.plasma;

import android.app.Activity;
import android.os.Bundle;
import android.content.Context;
import android.view.View;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.view.Display;
import android.view.WindowManager;
import android.view.View.OnClickListener;

import android.content.res.AssetManager;
import android.graphics.BitmapFactory;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.net.URL;

import com.example.plasma.R;

import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

import java.io.DataOutputStream;

import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import android.app.Activity;
import android.app.ProgressDialog;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;


public class Plasma extends Activity
{
    /** Called when the activity is first created. */
	ImageView imageview;
	private Bitmap input; 
	private Bitmap output; 
	private long mStartTime;
	final String localFileName = "local.jpg";
	final String remoteSrcFileName = "http://groups.csail.mit.edu/graphics/face/xform/uploads/input_image.jpg";
	
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imageview=(ImageView)findViewById(R.id.view1);
        input = getBitmapFromAsset(this, "6M.png");
        output = Bitmap.createBitmap(input.getWidth(), input.getHeight(), Bitmap.Config.ARGB_8888);
        imageview.setImageBitmap(input);
        mStartTime = System.currentTimeMillis();
    }
    
    /* Helpers*/
    private static Bitmap getBitmapFromAsset(Context context, String filePath) {
        AssetManager assetManager = context.getAssets();

        InputStream istr;
        Bitmap bitmap = null;
        try {
            istr = assetManager.open(filePath);
            bitmap = BitmapFactory.decodeStream(istr);
        } catch (IOException e) {
            // handle exception
        }

        return bitmap;
    }

    /* load our native library */
    static {

        //System.loadLibrary("plasma");
    	//System.loadLibrary("filter");
    	System.loadLibrary("native");
    	//System.loadLibrary("ndk1");
    }

    /* implementend by libplasma.so */
    //private static native void renderPlasma(Bitmap  bitmap, long time_ms);
    //private  native void boxblur(Bitmap bitmap);
    private  native void localLaplacian(Bitmap bitmap);
    //private native void helloLog(String logThis);
    
    // Buttons 
    public void blur(View view) {
        // Do something in respons	e to button
    	System.out.println("theButtonIsPressed");
    	//gradient();
    	//renderPlasma(output, System.currentTimeMillis() - mStartTime);
    	output = input; 
    	//boxblur(output);
    	//helloLog("This will log to LogCat via the native call.");
    	imageview.setImageBitmap(output);
    	input = output;   	
    }
    
    public void local_laplacian(View view){
    	try{
    	input = BitmapFactory.decodeStream(new FileInputStream(new File(getFilesDir(), localFileName)));
    	localLaplacian(input);
        FileOutputStream obuffer = new FileOutputStream(new File(getFilesDir(), localFileName));
        input.compress(Bitmap.CompressFormat.JPEG, 100, obuffer); 
        obuffer.close();
		}  catch (IOException e) {
            //You'll need to add proper error handling here
			e.printStackTrace();
    	}
    }

    public void updateImage(View v) {
        	try{
        		imageview.setImageBitmap(BitmapFactory.decodeStream(new FileInputStream(new File(getFilesDir(), localFileName))));
        	}catch (Exception e){
        		e.printStackTrace();
        	};
    }

    
    public void gradient(){
    	// Do something on mBitmap
    	int w = input.getWidth();
    	int h = input.getHeight();
    	int color1, color2;  
    	for(int i=0; i<h; i++){
    		for(int j=0; j<w-1; j++){
    			color1 = input.getPixel(j+1, i);		
    			color2 = input.getPixel(j, i);
    			output.setPixel(j,i, color1-color2);
    		}
    	}
    }
      

    public void downloadFile(View v){
   	
 	   try {
 		   	  String sourceFileUri = remoteSrcFileName;  		  
			   URL u = new URL(sourceFileUri);
			   InputStream is = u.openStream();

			   DataInputStream dis = new DataInputStream(is);

			   byte[] buff = new byte[10*1024*1024]; //Max 10 MB
			   int length;

	            FileOutputStream fos = new FileOutputStream(new File(getFilesDir(), localFileName));
	            while ((length = dis.read(buff))>0) {
	              fos.write(buff, 0, length);
	            }
			
	            fos.close();

		} catch (Exception e1) {

			e1.printStackTrace();
		}
 }
}



//Legacy
class PlasmaView extends View {
	private Bitmap input; //immutable
	private Bitmap output; // immutable
    private long mStartTime;

    /* implementend by libplasma.so */
    private static native void renderPlasma(Bitmap  bitmap, long time_ms);
    
    /* Helper to load image file */
    public static Bitmap getBitmapFromAsset(Context context, String filePath) {
        AssetManager assetManager = context.getAssets();

        InputStream istr;
        Bitmap bitmap = null;
        try {
            istr = assetManager.open(filePath);
            bitmap = BitmapFactory.decodeStream(istr);
        } catch (IOException e) {
            // handle exception
        }

        return bitmap;
    }

    public PlasmaView(Context context, int width, int height) {
        super(context);
        
        //mBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565)
        input = getBitmapFromAsset(context, "yichang.png");
        output = Bitmap.createBitmap(input.getWidth(), input.getHeight(), Bitmap.Config.ARGB_8888);
        gradient(); // Buffer the input gradient to output
        mStartTime = System.currentTimeMillis();
    }

    public void gradient(){
    	// Do something on mBitmap
    	int w = input.getWidth();
    	int h = input.getHeight();
    	int color1, color2;  
    	for(int i=0; i<h; i++){
    		for(int j=0; j<w-1; j++){
    			color1 = input.getPixel(j+1, i);		
    			color2 = input.getPixel(j, i);
    			output.setPixel(j,i, color1-color2);
    		}
    	}
    }    
    @Override protected void onDraw(Canvas canvas) {
        //canvas.drawColor(0xFFCCCCCC);
        //renderPlasma(mBitmap, System.currentTimeMillis() - mStartTime);
        canvas.drawBitmap(output, 0, 0, null);
        // force a redraw, with a different time-based pattern.
        //invalidate();
    }
}
