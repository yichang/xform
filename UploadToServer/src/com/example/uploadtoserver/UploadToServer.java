package com.example.uploadtoserver;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.DataInputStream;
import java.io.FileOutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import android.app.Activity;
import android.app.ProgressDialog;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

  
public class UploadToServer extends Activity {
     
    TextView messageText;
    ImageView currentImage;
    Button uploadButton;
    Button downloadInputButton;
    Button downloadResultButton;
    Button updateImageButton;
    Button reconInputButton;
    int serverResponseCode = 0;
    ProgressDialog dialog = null;
        
    String upLoadServerUri  = null;
    String recipeServerUri  = null;
    String upLoadServerRepo = null;
    String serverRoot       = null;
    
    Boolean Sleep_mode   = true;
    int Sleep_time_xform = 10;
    int Sleep_time_jpeg  = 500;
     
    /**********  File Path *************/
    final String localFileName     = "local.jpg";
    final String remoteSrcFileName = "http://bigboy.csail.mit.edu/xform_server/data/input_image.jpg";
     
    /* load our native library */
    static {
    	System.loadLibrary("xformRecon");
    } 

    private native void recon(Bitmap input, Bitmap ac_lumin, Bitmap ac_chrom, Bitmap dc, float[] bitmap);
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
         
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_upload_to_server);
          
        uploadButton         = (Button)findViewById(R.id.uploadButton);
        downloadInputButton  = (Button)findViewById(R.id.downloadInputButton);
        downloadResultButton = (Button)findViewById(R.id.downloadResultButton);
        updateImageButton    = (Button)findViewById(R.id.updateImage);
        reconInputButton     = (Button)findViewById(R.id.reconInputButton);
        messageText          = (TextView)findViewById(R.id.messageText);
        currentImage         = (ImageView)findViewById(R.id.currentImage);
         
        messageText.setText("Uploading file path :-" + localFileName );
         
        /************* PhP script path ****************/
        serverRoot = "http://bigboy.csail.mit.edu/xform_server/";
        
        if (Sleep_mode){
        	upLoadServerUri = serverRoot+"naive_upload";
        	recipeServerUri = serverRoot+"recipe_upload";       	
        } else{ 
        	/* upLoadServerUri = "http://groups.csail.mit.edu/graphics/face/xform/uploads.php"; */
        	/* recipeServerUri = "http://groups.csail.mit.edu/graphics/face/xform/recipe.php"; */
        }
        upLoadServerRepo = serverRoot+"uploads/"; 
        
        // Upload + Run + Download
        uploadButton.setOnClickListener(new OnClickListener() {            
            @Override
            public void onClick(View v) {

                dialog = ProgressDialog.show(UploadToServer.this, "", "Uploading compressed image...", true);

                new Thread(new Runnable() {
                    public void run() {
                        runOnUiThread(new Runnable() {
                            public void run() {
                                messageText.setText("uploading started.....");
                            }
                        });                

                        // Upload and run the image
                        uploadFile(localFileName, upLoadServerUri);

                        if (Sleep_mode){

                            runOnUiThread(new Runnable() {public void run() {                       
                                messageText.setText("Sleep mode started.... \n\n");
                            }});   
                            SystemClock.sleep(Sleep_time_jpeg);


                        }
                        runOnUiThread(new Runnable() {public void run() {                       
                            messageText.setText("Download started... \n\n");
                        }});  

                        // Download the result
                        downloadFile(upLoadServerRepo + localFileName, localFileName);  
                    }
                }).start();        
            }
        });
        
        // Upload + Fit recipe + Download
        reconInputButton.setOnClickListener(new OnClickListener() {            
            @Override
            public void onClick(View v) {
                 
                dialog = ProgressDialog.show(UploadToServer.this, "", "Play recipe...", true);
                 
                new Thread(new Runnable() {
                        public void run() {
                             runOnUiThread(new Runnable() {
                                    public void run() {
                                        messageText.setText("upload started.....");
                                    }
                                });                      
                           
                             // Degrade input
                             long upload_startTime = System.currentTimeMillis();
                             uploadFile(localFileName, recipeServerUri);
                             long upload_difference = System.currentTimeMillis() - upload_startTime;
                             
                             System.out.println("UPLOAD=" +  upload_difference + "ms");
                             
                             if (Sleep_mode){

                            	 runOnUiThread(new Runnable() {public void run() {                       
                            		 messageText.setText("Sleep mode started.... \n\n");
                                     }});   
                            	 SystemClock.sleep(Sleep_time_xform);             	 
                             }
                        	 runOnUiThread(new Runnable() {public void run() {                       
                        		 messageText.setText("Download mode started.... \n\n");
                        	 }});   
                        	 
                             long download_startTime = System.currentTimeMillis();                           
                             downloadFile(serverRoot + "output/recipe_ac_lumin.png", "recipe_ac_lumin.png"); 
                             downloadFile(serverRoot + "output/recipe_ac_chrom.png", "recipe_ac_chrom.png"); 
                             downloadFile(serverRoot + "output/recipe_dc.png", "recipe_dc.png"); 
                             downloadFile(serverRoot + "output/quant.meta", "quant.meta"); 
                             long download_difference = System.currentTimeMillis() - download_startTime;
                             System.out.println("DOWNLOAD=" +  download_difference + "ms");

                             runOnUiThread(new Runnable() {public void run() {                       
                           	 		messageText.setText("Reconstucting from recipe.... \n\n");
                           	 	}});   
                        	 	
                             try {
                            	final Bitmap input = BitmapFactory.decodeStream(new FileInputStream(new File(getFilesDir(), localFileName)));
								final Bitmap dc = BitmapFactory.decodeStream(new FileInputStream(new File(getFilesDir(), "recipe_dc.png")));
								final Bitmap ac_lumin = BitmapFactory.decodeStream(new FileInputStream(new File(getFilesDir(), "recipe_ac_lumin.png")));
								final Bitmap ac_chrom = BitmapFactory.decodeStream(new FileInputStream(new File(getFilesDir(), "recipe_ac_chrom.png")));
 
								// meta data
	                            File file = new File(getFilesDir(), "quant.meta");
	                            StringBuilder text = new StringBuilder();
                                BufferedReader br = new BufferedReader(new FileReader(file));
                                String line = br.readLine();
                                String[] ar=line.split(" ");
                                float[] myFloats = new float[ar.length];
                                for(int i=0; i < ar.length; i++)
                               	 myFloats[i] = Float.valueOf(ar[i]);
                             
                                recon(input, ac_lumin, ac_chrom, dc, myFloats );
                                
                                FileOutputStream out = new FileOutputStream(new File(getFilesDir(), localFileName));
                                input.compress(Bitmap.CompressFormat.JPEG, 100, out); 
                                out.close();                               
							}  catch (FileNotFoundException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							}  catch (IOException e) {
                                //You'll need to add proper error handling here
								e.printStackTrace();
                            }  catch (Exception e) {
                                e.printStackTrace();
                            }
                        	runOnUiThread(new Runnable() {public void run() {                       
                           	 		messageText.setText("Reconstruction Copmlete. \n\n");
                           	}});   
                            
                        }
                      }).start();        
                }
            });
        
        // Get a new input image from server
        downloadInputButton.setOnClickListener(new OnClickListener() {            
            @Override
            public void onClick(View v) {

                dialog = ProgressDialog.show(UploadToServer.this, "", "Download file...", true);

                new Thread(new Runnable() {
                    public void run() {
                        runOnUiThread(new Runnable() {
                            public void run() {
                                messageText.setText("downloading started.....");
                            }
                        });                      

                        //uploadFile(uploadFilePath + "" + uploadFileName);
                        downloadFile(remoteSrcFileName, localFileName);                                                      
                    }
                }).start();        
            }
        });

        downloadResultButton.setOnClickListener(new OnClickListener() {            
            @Override
            public void onClick(View v) {

                dialog = ProgressDialog.show(UploadToServer.this, "", "Download file...", true);

                new Thread(new Runnable() {
                    public void run() {
                        runOnUiThread(new Runnable() {
                            public void run() {
                                messageText.setText("downloading started.....");
                            }
                        });                      

                        //uploadFile(uploadFilePath + "" + uploadFileName);
                        downloadFile(upLoadServerRepo + localFileName, localFileName);                                                      
                    }
                }).start();        
            }
        });

        // Update the display
        updateImageButton.setOnClickListener(new OnClickListener() {            
            @Override
            public void onClick(View v) {
                try{
                    currentImage.setImageBitmap(BitmapFactory.decodeStream(new FileInputStream(new File(getFilesDir(), localFileName))));
                }catch (Exception e){
                    e.printStackTrace();
                };
            }
        });
    }
    
    public int downloadFile(String sourceFileUri, String localFileName){
    	   try {
   			   URL u = new URL(sourceFileUri);
   			   InputStream is = u.openStream();

   			   DataInputStream dis = new DataInputStream(is);

   			   byte[] buff = new byte[1024*1024]; //Max 1 MB
   			   int length;

   	            FileOutputStream fos = new FileOutputStream(new File(getFilesDir(), localFileName));
   	            while ((length = dis.read(buff))>0) {
   	              fos.write(buff, 0, length);
   	            }
   			
   	            fos.close();
   	            
                if(serverResponseCode == 200){
                    
                   Log.e("uploadFile", "SUCESSFUL download");
                    runOnUiThread(new Runnable() {
                         public void run() {                              
                             String msg = "Download Complete \n\n";                              
                             messageText.setText(msg);
                             Toast.makeText(UploadToServer.this, "File Download Complete.", 
                                          Toast.LENGTH_SHORT).show();
                         }
                     });                
                }   
   			
   			
   		} catch (Exception e1) {

   			e1.printStackTrace();
   		}
    	   
    	dialog.dismiss();  
    	return 0;
    }
      
    public int uploadFile(String sourceFileName, String dest) {
                     
          String fileName = sourceFileName;
  
          HttpURLConnection conn = null;
          DataOutputStream dos   = null;
          String lineEnd         = "\r\n";
          String twoHyphens      = "--";
          String boundary        = "--*****--";
          int bytesRead, bytesAvailable, bufferSize;
          byte[] buffer;
          int maxBufferSize = 1024 * 1024; // Max 1 MB
          File sourceFile = new File(getFilesDir(), sourceFileName);  
          
          if (!sourceFile.isFile()) {
               
               dialog.dismiss(); 
                
               Log.e("uploadFile", "Source File not exist :"
                                   + "" + localFileName);
                
               runOnUiThread(new Runnable() {
                   public void run() {
                       messageText.setText("Source File not exist :"
                              + localFileName);
                   }
               }); 
                
               return 0;
            
          }
          else
          {
               try { 
                    
                     // open a URL connection to the Servlet
                   FileInputStream fileInputStream = new FileInputStream(sourceFile);
                   URL url = new URL(dest);
                    
                   // Open a HTTP  connection to  the URL
                   conn = (HttpURLConnection) url.openConnection(); 
                   conn.setDoInput(true); // Allow Inputs
                   conn.setDoOutput(true); // Allow Outputs
                   conn.setUseCaches(false); // Don't use a Cached Copy
                   conn.setRequestMethod("POST");
                   conn.setRequestProperty("Connection", "Keep-Alive");
                   conn.setRequestProperty("ENCTYPE", "multipart/form-data");
                   conn.setRequestProperty("Content-Type", "multipart/form-data;boundary=" + boundary);
                   /* conn.setRequestProperty("uploaded_file", fileName);  */
                    
                   dos = new DataOutputStream(conn.getOutputStream());
          
                   /* dos.writeBytes(twoHyphens + boundary + lineEnd);  */
                   /* dos.writeBytes("Content-Disposition: form-data; name=\"uploaded_file\";filename=\"" */
                   /*                           + fileName + "\"" + lineEnd); */
                   dos.writeBytes(fileName);
                   dos.writeBytes(boundary); 
                   /* dos.writeBytes(lineEnd); */
          
                   // create a buffer of  maximum size
                   bytesAvailable = fileInputStream.available(); 
          
                   bufferSize = Math.min(bytesAvailable, maxBufferSize);
                   buffer = new byte[bufferSize];
          
                   // read file and write it into form...
                   bytesRead = fileInputStream.read(buffer, 0, bufferSize);  
                      
                   while (bytesRead > 0) {
                        
                     dos.write(buffer, 0, bufferSize);
                     bytesAvailable = fileInputStream.available();
                     bufferSize = Math.min(bytesAvailable, maxBufferSize);
                     bytesRead = fileInputStream.read(buffer, 0, bufferSize);   
                      
                    }
          
                   // send multipart form data necesssary after file data...
                   /* dos.writeBytes(lineEnd); */
                   /* dos.writeBytes(twoHyphens + boundary + twoHyphens + lineEnd); */
                   /* dos.writeBytes(twoHyphens + boundary + twoHyphens + lineEnd); */
          
                   // Responses from the server (code and message)
                   serverResponseCode = conn.getResponseCode();
                   String serverResponseMessage = conn.getResponseMessage();
                     
                   Log.i("uploadFile", "HTTP Response is : "
                           + serverResponseMessage + ": " + serverResponseCode);
                    
                   if(serverResponseCode == 200){
                        
                       runOnUiThread(new Runnable() {
                            public void run() {
                                 
                                String msg = "File Upload Completed.\n\n See uploaded file here : \n\n"
                                              + upLoadServerRepo 
                                              + localFileName;
                                 
                                messageText.setText(msg);
                                Toast.makeText(UploadToServer.this, "File Upload Complete.", 
                                             Toast.LENGTH_SHORT).show();
                            }
                        });                
                   }    
                    
                   //close the streams //
                   fileInputStream.close();
                   dos.flush();
                   dos.close();
                     
              } catch (MalformedURLException ex) {
                   
                  dialog.dismiss();  
                  ex.printStackTrace();
                   
                  runOnUiThread(new Runnable() {
                      public void run() {
                          messageText.setText("MalformedURLException Exception : check script url.");
                          Toast.makeText(UploadToServer.this, "MalformedURLException", 
                                                              Toast.LENGTH_SHORT).show();
                      }
                  });
                   
                  Log.e("Upload file to server", "error: " + ex.getMessage(), ex);  
              } catch (Exception e) {
                   
                  dialog.dismiss();  
                  e.printStackTrace();
                   
                  runOnUiThread(new Runnable() {
                      public void run() {
                          messageText.setText("Got Exception : see logcat ");
                          Toast.makeText(UploadToServer.this, "Got Exception : see logcat ", 
                                  Toast.LENGTH_SHORT).show();
                      }
                  });
                  Log.e("Upload file to server Exception", "Exception : "
                                                   + e.getMessage(), e);  
              }
              dialog.dismiss();       
              return serverResponseCode; 
               
           } // End else block 
         } 
}
