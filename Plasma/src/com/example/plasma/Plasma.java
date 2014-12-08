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

import android.content.res.AssetManager;
import android.graphics.BitmapFactory;
import java.io.InputStream;
import java.io.IOException;

import com.example.plasma.R;
import android.widget.ImageView;

public class Plasma extends Activity
{
    /** Called when the activity is first created. */
	ImageView imageview;
	private Bitmap input; 
	private Bitmap output; 
	private long mStartTime;
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        //Display display = getWindowManager().getDefaultDisplay().;
        //setContentView(new PlasmaView(this, display.getWidth(), display.getHeight()));
        //setContentView(new PlasmaView(this, 100, 100));
        setContentView(R.layout.activity_main);
        imageview=(ImageView)findViewById(R.id.view1);
        input = getBitmapFromAsset(this, "yichang.png");
        output = Bitmap.createBitmap(input.getWidth(), input.getHeight(), Bitmap.Config.RGB_565);
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

        System.loadLibrary("plasma");
    }

    /* implementend by libplasma.so */
    private static native void renderPlasma(Bitmap  bitmap, long time_ms);
    
    public void sendMessage(View view) {
        // Do something in response to button
    	System.out.println("theButtonIsPressed");
    	gradient();
    	//renderPlasma(output, System.currentTimeMillis() - mStartTime);
    	imageview.setImageBitmap(output);
    	input = output;   	
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
