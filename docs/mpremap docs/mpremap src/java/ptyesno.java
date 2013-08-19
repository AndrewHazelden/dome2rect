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

import java.awt.*;
import java.awt.event.*;


class ptyesno extends Dialog implements ActionListener {
	public boolean result = true;
	Button yes, no;
	
	
    public ptyesno(Frame dw, String text) {
        super(dw, "", true);


        Panel p1 = new Panel();
        TextArea t = new TextArea( text, 3, 40, TextArea.SCROLLBARS_NONE);
        t.setEditable(false);
        p1.add(t);
        add("Center", p1);


       	Panel p2 = new Panel();
        p2.setLayout(new FlowLayout(FlowLayout.CENTER));
        yes = new Button("yes");
        yes.addActionListener(this);
       	p2.add(yes);
        no = new Button("no");
        no.addActionListener(this);
       	p2.add(no);
       	add("South", p2);

        pack();
    }

    public void actionPerformed(ActionEvent event) {
    	if( event.getSource() == no )
    		result = false;
        setVisible(false);
    }
}
