package com.example.uploadtoserver;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.DataInputStream;
import java.io.FileOutputStream;
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

  
public class UploadToServer extends Activity {
     
    TextView messageText;
    ImageView currentImage;
    Button uploadButton;
    Button downloadInputButton;
    Button downloadResultButton;
    Button updateImageButton;
    int serverResponseCode = 0;
    ProgressDialog dialog = null;
        
    String upLoadServerUri = null;
    String upLoadServerRepo = null;
     
    /**********  File Path *************/
    final String localFileName = "local.jpg";
    final String remoteSrcFileName = "http://people.csail.mit.edu/yichangshih/6M.jpg";
     
    @Override
    public void onCreate(Bundle savedInstanceState) {
         
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_upload_to_server);
          
        uploadButton = (Button)findViewById(R.id.uploadButton);
        downloadInputButton = (Button)findViewById(R.id.downloadInputButton);
        downloadResultButton = (Button)findViewById(R.id.downloadResultButton);
        updateImageButton = (Button)findViewById(R.id.updateImage);
        messageText  = (TextView)findViewById(R.id.messageText);
        currentImage = (ImageView)findViewById(R.id.currentImage);
         
        messageText.setText("Uploading file path :-" + localFileName );
         
        /************* PhP script path ****************/
        upLoadServerUri = "http://groups.csail.mit.edu/graphics/face/xform/uploads.php";
        upLoadServerRepo = "http://groups.csail.mit.edu/graphics/face/xform/uploads/"; 
        uploadButton.setOnClickListener(new OnClickListener() {            
            @Override
            public void onClick(View v) {
                 
                dialog = ProgressDialog.show(UploadToServer.this, "", "Uploading file...", true);
                 
                new Thread(new Runnable() {
                        public void run() {
                             runOnUiThread(new Runnable() {
                                    public void run() {
                                        messageText.setText("uploading started.....");
                                    }
                                });                      
                           
                             //uploadFile(uploadFilePath + "" + uploadFileName);
                             //downloadFile(remoteSrcFileName, localFileName);
                             uploadFile(localFileName);
                                                      
                        }
                      }).start();        
                }
            });
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
    	   
    	dialog.dismiss();  
    	return 0;
    }
      
    public int uploadFile(String sourceFileName) {
                     
          String fileName = sourceFileName;
  
          HttpURLConnection conn = null;
          DataOutputStream dos = null;  
          String lineEnd = "\r\n";
          String twoHyphens = "--";
          String boundary = "*****";
          int bytesRead, bytesAvailable, bufferSize;
          byte[] buffer;
          int maxBufferSize = 10 * 1024 * 1024; // Max 10 MB
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
                   URL url = new URL(upLoadServerUri);
                    
                   // Open a HTTP  connection to  the URL
                   conn = (HttpURLConnection) url.openConnection(); 
                   conn.setDoInput(true); // Allow Inputs
                   conn.setDoOutput(true); // Allow Outputs
                   conn.setUseCaches(false); // Don't use a Cached Copy
                   conn.setRequestMethod("POST");
                   conn.setRequestProperty("Connection", "Keep-Alive");
                   conn.setRequestProperty("ENCTYPE", "multipart/form-data");
                   conn.setRequestProperty("Content-Type", "multipart/form-data;boundary=" + boundary);
                   conn.setRequestProperty("uploaded_file", fileName); 
                    
                   dos = new DataOutputStream(conn.getOutputStream());
          
                   dos.writeBytes(twoHyphens + boundary + lineEnd); 
                   dos.writeBytes("Content-Disposition: form-data; name=\"uploaded_file\";filename=\""
                                             + fileName + "\"" + lineEnd);
                    
                   dos.writeBytes(lineEnd);
          
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
                   dos.writeBytes(lineEnd);
                   dos.writeBytes(twoHyphens + boundary + twoHyphens + lineEnd);
          
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