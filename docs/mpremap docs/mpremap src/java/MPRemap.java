/* MPRemap 
   Copyright (C) 2007 - Helmut Dersch  der@fh-furtwangen.de
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/*------------------------------------------------------------*/


import java.io.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;


public class MPRemap extends Frame implements ActionListener, Runnable{
	static final boolean debug = true;

	private Button ok,cancel;
	private TextArea status;
	private Properties prefs;
	
	
	private String[] pref_names={"input", "output", 
								 "mpremap", "script", "extract", "quality", "motionblur", 
								 "ffmpeg", "use_ffmpeg_for_input", "deinterlace", 
								 "use_ffmpeg_for_output", "output_format", "audio",
								 "editor"};
	private String[] pref_vals ={"","",
								 "","","true","80","1",
								 "","false","true", 
								 "false"," -b 2000kb  -vcodec mpeg1video ", " -acodec copy",
								 ""};
	private String preferences="MPRemap.preferences";
	private String version="MPRemap Version 0.2a";
	private Thread mpr = null;
	private Process pr;
	private MenuItem m11,m12,m13,m21, m22, m23, m24, m25,m31,
	                 m32,m33,m41,m42,m43,m44,m45,m46,m47,m48,m49,m50;
	private CheckboxMenuItem c21,c31,c32,c33;


	void sprintPreferences(){
		setPrefs();
		sprintln("Options set to:");
		for(int i=0; i<pref_names.length; i++){
			sprintln("  "+pref_names[i]+"="+prefs.getProperty(pref_names[i]));
		}
		sprintln("");
	}
	
	void sprintOption(String key){
		sprintln(key+" set to \"" + prefs.getProperty(key)+"\"");
	}
	
	void sprintResource(String rname){
		try{
			InputStream in = ClassLoader.getSystemResourceAsStream(rname);
			String line;
			while((line=readLine(in)) != null)
				sprintln(line);
			in.close();
		}catch(Exception e){
			sprintln("Could not open resource "+rname);
		}
	}
			

	private void stopConversion(){
		try{
			if(pr instanceof Process)
				pr.destroy();
			if(mpr != null && mpr.isAlive())
				mpr.join();
		}catch(Exception e){
		}
		mpr = null;
	}
				
		
	private void getPrefs(){
		prefs = new Properties();
		for(int i=0; i<pref_names.length; i++)
			prefs.setProperty(pref_names[i], pref_vals[i]);
		try{
			FileInputStream f = new FileInputStream(preferences);
			prefs.load(f);
			f.close();
		}catch(Exception e){ // preferences file does not exist; no error
		}
	}
	

	
	private void setPrefs(){
		prefs.setProperty("deinterlace", 	c32.getState()?"true":"false");
		prefs.setProperty("extract", c21.getState()?"true":"false");
		prefs.setProperty("use_ffmpeg_for_input", c31.getState()?"true":"false");
		prefs.setProperty("use_ffmpeg_for_output", c33.getState()?"true":"false");
	}


	private void savePrefs(){
		setPrefs();
		try{
			FileOutputStream f = new FileOutputStream(preferences);
			prefs.store(f,"");
			f.close();
		}catch(Exception e){
		}
	}
	
	private void createGUI(){
		setSize(600,400);
		setTitle("Remap from/to motion picture");
			// Create GUI
		MenuBar mb = new MenuBar();
		Menu m = new Menu("File");
		m11 = new MenuItem("Input Video/Image", new MenuShortcut(KeyEvent.VK_O, false));  
		m11.addActionListener(this); 
		m.add(m11);		
		m12 = new MenuItem("Save as", new MenuShortcut(KeyEvent.VK_S, false)); 
		m12.addActionListener(this);
		m.add(m12);
		m.addSeparator();
		m13 = new MenuItem("Exit", new MenuShortcut(KeyEvent.VK_Q, false)); 
		m13.addActionListener(this);
		m.add(m13);	
		mb.add(m);
		
		m = new Menu("Mpremap");
		m21 = new MenuItem("Path to Mpremap", new MenuShortcut(KeyEvent.VK_M, false)); 
		m21.addActionListener(this);
		m.add(m21);
		c21 = new CheckboxMenuItem("Extract", prefs.getProperty("extract").equals("true"));
		m.add(c21);
		m.addSeparator();
		m22 = new MenuItem("Conversion Script", new MenuShortcut(KeyEvent.VK_C, false)); 
		m22.addActionListener(this);
		m.add(m22);
		m23 = new MenuItem("Edit Script", new MenuShortcut(KeyEvent.VK_E, false)); 
		m23.addActionListener(this);
		m.add(m23);
		m.addSeparator();
		m24 = new MenuItem("JPEG-Quality", new MenuShortcut(KeyEvent.VK_J, false)); 
		m24.addActionListener(this);
		m.add(m24);
		m25 = new MenuItem("Motion Blur", new MenuShortcut(KeyEvent.VK_B, false)); 
		m25.addActionListener(this);
		m.add(m25);
		mb.add(m);
		
		m = new Menu("Ffmpeg");
		m31 = new MenuItem("Path to Ffmpeg", new MenuShortcut(KeyEvent.VK_F, false)); 
		m31.addActionListener(this);
		m.add(m31);
		c31 = new CheckboxMenuItem("Input via Ffmpeg", 
					prefs.getProperty("use_ffmpeg_for_input").equals("true"));
		m.add(c31);		
		c32 = new CheckboxMenuItem("Deinterlace", 
					prefs.getProperty("deinterlace").equals("true"));
		m.add(c32);	
		m.addSeparator();
		c33 = new CheckboxMenuItem("Output via Ffmpeg", 
					prefs.getProperty("use_ffmpeg_for_output").equals("true"));
		m.add(c33);
		m32 = new MenuItem("Video Options", new MenuShortcut(KeyEvent.VK_V, false)); 
		m32.addActionListener(this);
		m.add(m32);		
		m33 = new MenuItem("Audio Options", new MenuShortcut(KeyEvent.VK_A, false)); 
		m33.addActionListener(this);
		m.add(m33);				
		mb.add(m);
		
		m = new Menu("Help");
		m41 = new MenuItem("General");
		m41.addActionListener(this);
		m.add(m41);		
		m.addSeparator();
		m.add("Menus:");
		m42 = new MenuItem("File");
		m42.addActionListener(this);
		m.add(m42);		
		m43 = new MenuItem("Mpremap");
		m43.addActionListener(this);
		m.add(m43);		
		m44 = new MenuItem("Mpremap - Script");
		m44.addActionListener(this);
		m.add(m44);		
		m45 = new MenuItem("Ffmpeg");
		m45.addActionListener(this);
		m.add(m45);	
		m.addSeparator();
		m.add("Helpers:");
		m46 = new MenuItem("mpremap");
		m46.addActionListener(this);
		m.add(m46);	
		m47 = new MenuItem("ffmpeg-general");
		m47.addActionListener(this);
		m.add(m47);	
		m48 = new MenuItem("ffmpeg-video");
		m48.addActionListener(this);
		m.add(m48);	
		m49 = new MenuItem("ffmpeg-audio");
		m49.addActionListener(this);
		m.add(m49);	
		m.addSeparator();	
		m50 = new MenuItem("About"); 
		m50.addActionListener(this);
		m.add(m50);	
		mb.add(m);	
		
		setMenuBar(mb);
		 
		
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS)); 

		status = new TextArea(version+"\n",8,200);
		status.setEditable(false);
		add(status);


		Panel p=new Panel();		
		ok = new Button("Start");
		ok.addActionListener(this);
		p.add(ok);
		cancel= new Button("Cancel");
		cancel.addActionListener(this);
		p.add(cancel);
		add(p);
	}

	
	private boolean filedialog(String key, String s, boolean load){
		FileDialog fd = new FileDialog(this, s, 
					load ? FileDialog.LOAD: FileDialog.SAVE);
		File f = new File(prefs.getProperty(key));
		try{
			f = f.getCanonicalFile();
		}catch(Exception e){
		}
		fd.setFile(f.getName());
		if(f.getParent() != null)
			fd.setDirectory(f.getParent());
		fd.show();
		if(fd.getFile() != null){
			prefs.setProperty(key, fd.getDirectory()+fd.getFile());
			return true;
		}else
			return false;				  
	}

	private boolean filedialog(String key, String s){
		return filedialog( key, s, true);
	}

	
	private boolean textdialog(String key, String query){
		pttext pt = new pttext(this, "", query, prefs.getProperty(key));
		pt.show();
		if(pt.result){
			prefs.setProperty(key, pt.field.getText());
			return true;
		}else{
			return false;
		}
	}

	
	public void actionPerformed(ActionEvent e){
		 Object s = e.getSource();
		 if(s==m11){
		 	if(filedialog("input", "Find input video/image"))
				sprintOption("input");
		 }else if(s==m12){
		 	if(filedialog("output", "Set output", false))
				sprintOption("output");				
		 }else if(s==m21){
		 	if(filedialog("mpremap", "Find Mpremap application"))
				sprintOption("mpremap");								
		 }else if(s==m22){
		 	if(filedialog("script", "Find conversion script")){
				try{
					FileInputStream f = new FileInputStream( prefs.getProperty("script" ));
					String line;
					
					sprintln("Script \""+prefs.getProperty("script" )+"\":");
					while( (line=readLine(f)) != null )
						sprintln(line);
					f.close();
				}catch(Exception ex){
					sprintln("Could not read script "+prefs.getProperty("script" )+".");
					printStackTrace(ex);
				}
			}
		 }else if(s==m23){
		 	if( "".equals(prefs.getProperty("editor"))){
				if(!filedialog("editor", "Find your favourite plain-text editor"))
					return;
				sprintOption("editor");
			}			
		 	try{
		 		String cmd =  prefs.getProperty("editor");
				cmd += " ";
				cmd += prefs.getProperty("script" );
				Runtime.getRuntime().exec( cmd );
			}catch(Exception er){
				sprintln("Error calling "+prefs.getProperty("editor")+" :"+er.toString());
			}
		 }else if(s==m24){
		 	if(textdialog("quality", "Set JPEG quality (0...100)"))
				sprintOption("quality");								
		 }else if(s==m25){
		 	if(textdialog("motionblur", "Motion Blur (>=1)"))
				sprintOption("motionblur");								
		 }else if(s==m31){
		 	if(filedialog("ffmpeg", "Find Ffmpeg application"))
				sprintOption("ffmpeg");								
		 }else if(s==m32){
		 	if(textdialog("output_format", "Set video options:"))
				sprintOption("output_format");												
		 }else if(s==m33){
		 	if(textdialog("audio", "Set audio options:"))
				sprintOption("audio");																
		 }else if(s==m41){
		 		sprintResource("help/Mpremap_general");																
		 }else if(s==m42){
		 		sprintResource("help/Mpremap_file");																
		 }else if(s==m43){
		 		sprintResource("help/Mpremap_mpremap");																
		 }else if(s==m44){
		 		sprintResource("help/mpremap_script");																
		 }else if(s==m45){
		 		sprintResource("help/Mpremap_ffmpeg");																
		 }else if(s==m46){
		 		sprintResource("help/mpremap");																
		 }else if(s==m47){
		 		sprintResource("help/ffmpeg_1");																
		 }else if(s==m48){
		 		sprintResource("help/ffmpeg_2");																
		 }else if(s==m49){
		 		sprintResource("help/ffmpeg_3");																
		 }else if(s==m50){
		 		sprintln(version);
				sprintLicense();
				sprintPreferences();																
		 }else if(s==ok){
		 	try{
		 		savePrefs();
				File dest = new File( prefs.getProperty("output") );
				if( dest.exists() && !mayOverwrite( dest ))
						return;
	
				mpr = new Thread( this );
				mpr.start();
			}catch(Exception ex){
				printStackTrace(ex);
			}
		 }else if(s==m13 || s==cancel){
		 	stopConversion();
		 	System.exit(0);
		}
	}
	
	
	private void printStackTrace(Exception e){
		sprintln("Error: "+e.toString());
		sprintln("Stacktrace: ");				
		StackTraceElement[] st = e.getStackTrace();
		if(st != null){
			for(int i=0; i<st.length; i++)
				sprintln(i + ": "+st[i].toString());
		}
	}		
	
	private boolean mayOverwrite( File f ){
		ptyesno pt = new ptyesno( this, f.getName() +" exists, overwrite?" );
		pt.show();
		return pt.result;
	}
	
	
	private void pipe(Process from, Process to) throws Exception{
		InputStream in = from.getInputStream();
		OutputStream out = to.getOutputStream();
		
		int len;
		byte[] buf = new byte[1024];
		while((len=in.read(buf,0,buf.length))>0){
			out.write(buf,0,len);
		}

		out.write(-1); // Braindead Windows requires this.
		out.flush();
		out.close();
		in.close();
	}
	

	static final int MAXTEXTLENGTH = 20000;

	private void sprintwrap(String s){
		int i=0,len = 50;

		if(s.length()<=len)
			sprintln(s);
		else{
			sprintln(s.substring(0,len)+'\\');
			sprintwrap(s.substring(len));
		}
	}
		
	
	private synchronized void sprintln(String s){

//		synchronized(status){
			try{
				String t = status.getText();
				if( t.length() > MAXTEXTLENGTH)
					status.setText( t.substring(MAXTEXTLENGTH/2)+s+'\n');
				else
					status.append(s+'\n');
			}catch(Exception e){ // textbuffer overflow
				status.setText(s+"\n");
			}
			status.repaint();
//		}
	}
	
	class PipeRunner implements Runnable{
		Process p1, p2;
		
		public PipeRunner(Process p1, Process p2){
			this.p1 = p1;
			this.p2 = p2;
		}
		
		public void run(){
			try{
				pipe(p1,p2);
			}catch(Exception e){
				sprintln("Error in pipe:");
				printStackTrace(e);
			}
		}
	}
		
	
	class PrintRunner implements Runnable{
		InputStream in=null;
		
		public PrintRunner(InputStream in){
			this.in = in;
		}
	
		public void run(){		
			try{
				if(in != null){
					String s=null;
					while((s=readLine(in)) !=null){
						sprintln(s);
					}
				}
			}catch(Exception e){
				sprintln("Error: "+e.toString());
			}

		}
	}

	public String readLine(InputStream in) throws Exception{
		StringBuffer sb = new StringBuffer();
		int c;
		// Skip leading crlf
		while( (c=in.read()) !=-1 && (c==0x0d || c==0x0a) );
		
		if(c==-1)
			return null;
		sb.append((char)c);
		while( (c=in.read()) !=-1){
			if(c==0x0d || c==0x0a)
				break;
			sb.append((char)c);
		}
		return sb.toString();
	}
		
	public void run(){
		try{
			cconvert();
		}catch(Exception e){
			printStackTrace(e);
		}			
	}
	
	private String quote(String s){
		return s;//'\"'+s+'\"';
	}	

	private String cmdVideo() throws Exception{
		String cmd = "";
		// ffmpeg reader
		if("true".equals( prefs.getProperty("use_ffmpeg_for_input") )){
			cmd += prefs.getProperty("ffmpeg");
			if("true".equals(prefs.getProperty("deinterlace")))
				cmd += " -deinterlace";
			cmd += " -i " + quote(prefs.getProperty("input"));
			cmd += " -vcodec ppm -f image2pipe - |";
		}
		
		// mpremap
		cmd += prefs.getProperty("mpremap");
		cmd += " -f "+ quote(prefs.getProperty("script"));
		if("true".equals( prefs.getProperty("extract") ))
			cmd += " -e ";
		cmd += " -q " + prefs.getProperty("quality");
		cmd += " -b " + prefs.getProperty("motionblur");
		if( "true".equals(prefs.getProperty("use_ffmpeg_for_output") ))
			cmd += " -o - ";
		else
			cmd += " -o " + quote(prefs.getProperty("output"));
		if( "true".equals(prefs.getProperty("use_ffmpeg_for_input") ))
			cmd += " - ";
		else
			cmd += " " + quote(prefs.getProperty("input"));

		// ffmpeg writer
		if( "true".equals(prefs.getProperty("use_ffmpeg_for_output") )){
			cmd += " | ";
			cmd += prefs.getProperty("ffmpeg");
			cmd += " -vcodec ppm  -f image2pipe -i  - ";
			cmd += prefs.getProperty("output_format");
			cmd += " -y " + quote(prefs.getProperty("output"));
		}
		return cmd;
	}
	
	private String escape(String s){
		char esc = '$';
		String os = System.getProperty("os.name").toLowerCase();
		if(os.indexOf("windows")>=0 || os.indexOf("vista")>=0)
			esc = '%';
			
		String r = "";
		for(int i=0; i<s.length(); i++){
			r += s.charAt(i);
			if(s.charAt(i) == esc){
				r += esc;
			}
		}
		return r;
	}
			
		

	private void execScript( String cmd ) throws Exception{
		String cmdfile="mpr_cmd.bat";
		String os = System.getProperty("os.name").toLowerCase();
		
		// Save command as textfile
		FileOutputStream f = new FileOutputStream(cmdfile);
		f.write( escape(cmd).getBytes() ) ;
		f.close();
		
		sprintln("Executing:");
		sprintwrap(cmd);
		if(os.indexOf("windows")>=0 || os.indexOf("vista")>=0){
			pr = Runtime.getRuntime().exec( cmdfile );
		}else 
			pr = Runtime.getRuntime().exec( "/bin/sh " + cmdfile );		
		//throw new Exception("Unknown OS: "+os);
		(new Thread( new PrintRunner(pr.getErrorStream()))).start();
		int e = pr.waitFor();
		sprintln("Process finished with status: "+e);
		pr = null;
		if(!debug)
			(new File(cmdfile)).delete();
		return;
	}	
		

			

	private void cconvert() throws Exception{
		int e0,e1;
		boolean err=false;
		
		// Make a few checks
		File fc;
		fc = new File(prefs.getProperty("input"));
		if(!fc.exists()){
			if(prefs.getProperty("input").indexOf('%') == -1){
				sprintln("Please set input image or videofile.");
				err=true;
			}
		}
		if("".equals(prefs.getProperty("output").trim())){
			sprintln("Please set output file.");
			err=true;
		}
		fc = new File(prefs.getProperty("mpremap"));
		if(!fc.exists()){
			sprintln("Please find Mpremap application.");
			err=true;
		}
		fc = new File(prefs.getProperty("script"));
		if(!fc.exists()){
			sprintln("Please find conversion script.");
			err=true;
		}
		
		if( "true".equals( prefs.getProperty("use_ffmpeg_for_input") )
			|| "true".equals(prefs.getProperty("use_ffmpeg_for_output") )){
				fc = new File(prefs.getProperty("ffmpeg"));
				if(!fc.exists()){
					sprintln("Please find Ffmpeg application.");
					err=true;
				}
		}
		if(err) return;

		// Start video process	
		execScript( cmdVideo() );
		
		
		// Add audio if required and available
		if("true".equals( prefs.getProperty("use_ffmpeg_for_input" )) &&
		   "true".equals( prefs.getProperty("use_ffmpeg_for_output")) && 
		   !"".equals(prefs.getProperty("audio").trim())){
			String ftmp = prefs.getProperty("output");
			ftmp = ftmp.substring(0, ftmp.lastIndexOf('.')) + 
					"_temp" + ftmp.substring( ftmp.lastIndexOf('.'));
			File fd = new File(prefs.getProperty("output"));
			fd.renameTo( new File(ftmp));
			
			String cmd = prefs.getProperty("ffmpeg");
			cmd += " -i "+ ftmp;
			cmd += " -i " + prefs.getProperty("input");
			cmd += " -vcodec copy ";
			cmd += prefs.getProperty("audio");
			cmd += " -y ";
			cmd += prefs.getProperty("output");
			cmd += " -map 0:0 -map 1:1";
			execScript( cmd );
			(new File(ftmp)).delete();
		}
				
		sprintln("Conversion finished.");
	}




	public MPRemap() throws Exception{
		getPrefs();
		createGUI();
		addWindowListener(new WindowAdapter(){
			public void windowClosing(WindowEvent e){
				stopConversion();
				System.exit(0);
			}});
		show();
		sprintLicense();
		sprintln("");
		sprintPreferences();		
	}
	
	public static void main(String[] args){
		try{
			new MPRemap();
		}catch(Exception e){
			System.out.println("Error: "+e.toString());
			e.printStackTrace();
		}
	}
	
	void sprintLicense(){
		sprintln("Copyright (C) 2007 - Helmut Dersch  der@fh-furtwangen.de");
		sprintln("");
		sprintln("This program is free software; you can redistribute it and/or modify");
		sprintln("it under the terms of the GNU General Public License as published by");
		sprintln("the Free Software Foundation; either version 2, or (at your option)");
		sprintln("any later version.");
		sprintln("");
		sprintln("This program is distributed in the hope that it will be useful,");
		sprintln("but WITHOUT ANY WARRANTY; without even the implied warranty of");
		sprintln("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the");
		sprintln("GNU General Public License for more details.");
		sprintln("");
		sprintln("You should have received a copy of the GNU General Public License");
		sprintln("along with this program; if not, write to the Free Software");
		sprintln("Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.");
		sprintln("");
	}
}
