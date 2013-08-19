/* 
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


class pttext extends Dialog implements ActionListener {
    public TextField field;
    Button 	setButton;
    boolean result = false;

    public pttext(Frame dw, String title, String query, String deftext){
        super(dw, "", true);
		
		// Create GUI
		
        //Create middle section.
        Panel p1 = new Panel();
        Label label = new Label(query);
        p1.add(label);
        field = new TextField( deftext, 20);
        field.addActionListener(this);
        p1.add(field);
        add("Center", p1);

        //Create bottom row.
        Panel p2 = new Panel();
        p2.setLayout(new FlowLayout(FlowLayout.CENTER));
        Button b = new Button("Cancel");
        b.addActionListener(this);
        setButton = new Button("OK");
        setButton.addActionListener(this);
        p2.add(b);
        p2.add(setButton);
        add("South", p2);

        //Initialize this dialog to its preferred size.
        pack();
    }

    public void actionPerformed(ActionEvent event) {
        Object source = event.getSource();
        if ( (source == setButton)
           | (source == field)) {
           	result = true;
        }
        setVisible(false);
    }
}
