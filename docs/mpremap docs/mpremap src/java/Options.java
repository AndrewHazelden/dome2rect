/* CConvert - convert video to MJPEG-format viewable on Canon cameras
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
import java.util.*;

public	class Options{
		Enumeration oc;
		Hashtable ho;
		public String optArg=null;
		public char   opt='0';
		public String args[];

		public Options(String opt, String[] args) throws Exception{
			Vector argsv=new Vector();
			ho = new Hashtable();
			for(int i=0; i<args.length; i++)
				argsv.addElement(args[i]);
			for(int i=0; i<opt.length(); i++){
			 	char c = opt.charAt(i);
				String sc = "-"+c;
				for(int k=0; k<argsv.size(); k++){
					String arg = (String)argsv.elementAt(k);
					if(arg.equals(sc)){
						argsv.removeElementAt(k);
						Object optstr = "";
						if(i<opt.length()-1 && opt.charAt(i+1)==':'){
							if(k==argsv.size())
								throw new Exception
									("Option String for Option "+c+" missing.");
							optstr=argsv.elementAt(k);
							argsv.removeElementAt(k);
							i++;
						}
						ho.put(new Character(c), optstr);
					}
				}
			}
			oc = ho.keys();
			this.args = new String[argsv.size()];
			for(int i=0; i<argsv.size(); i++)
				this.args[i]=(String)argsv.elementAt(i);
		}
		
		public boolean getOption(){
			if(oc.hasMoreElements()){
				Character c = (Character)oc.nextElement();
				optArg = (String)ho.get(c);
				opt = c.charValue();
				return true;
			}
			return false;
		}
	}		
