/********************************************************************
Copyright (C) 2016 Martin Ryberg

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

contact: martin.ryberg@ebc.uu.se
*********************************************************************/

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <ctime>
#include <vector>
#include "pugixml/pugixml.hpp"
#include "drawing.h"

using namespace std;

/*void sleep(unsigned int mseconds) {
    clock_t goal = mseconds + clock();
    while (goal > clock());
}*/

void close_x(Display* dis, Window &win, GC &gc) {
    XFreeGC(dis, gc);
    XDestroyWindow(dis, win);
    XCloseDisplay(dis);
}

void display_drawing(Display* dis, Window& win, GC& gc, drawing& fig, int x_setoff, int y_setoff, float scale); 
bool pars_svg(istream& input, drawing& fig);
void help();
int main (int argc, char *argv[]) {
    // Variables for general behaviour
    bool quiet(true);
    string file_name;
    ifstream infile;
    istream* input = &cin;
    // Pars arguments
    for (int i=1; i < argc; ++i) {
	if ( !strcmp(argv[i],"-f") || !strcmp(argv[i],"--file") ) {
	    if (i+1 < argc && argv[i+1][0] != '-') {
		file_name = argv[++i];
	    }
	    else {
		cerr << "-f/--file needs to be followed by a file name." << endl;
		return 1;
	    }
	}
	else if (!strcmp(argv[i],"-v") || !strcmp(argv[i],"--verbose")) quiet = false;
	else if ( !strcmp(argv[i],"-h") || !strcmp(argv[i],"--help") ) {
	    help();
	    return 0;
	}
	else if ( i == argc-1 && argv[i][0] != '-' && file_name.empty() ) {
	    file_name = argv[i];
	}
	else if (i < argc) {
	    std::cerr << "The program was called with the following command:" << endl;
	    for (int j=0; j<argc; ++j) std::cerr << argv[j] << ' ';
	    std::cerr << endl;
	    std::cerr << "Argument " << argv[i] << " not recognized. For available arguments give -h or --help." << endl;
	    return 0;
	}

    }
    // Open infile if one given
    if (!file_name.empty()) {
	infile.open(file_name.c_str(), std::ifstream::in);
	if (infile.good())
	    input = &infile;
	else {
	    cerr << "Could not open file: " << file_name << endl;
	    return 1;
	}
    }

    // Pars SVG
    drawing figure;
    if (pars_svg(*input, figure) && !quiet) std::cerr << "Parsed figure." << std::endl;
    else if (!quiet) {
        std::cerr << "Unable to pars figure." << std::endl;
	return 1;
    }

    // Graphical variables
    Display* dis;
    int screen;
    Window win;
    XEvent report;
    GC green_gc;

    // Prepair window
    #ifdef DEBUG
    cerr << "Opening display" << endl;
    #endif // DEBUG
    dis = XOpenDisplay(NULL);
    screen = DefaultScreen(dis);

    // Prepair colors
    unsigned long black;//, white; 
    black = BlackPixel(dis, screen);
    //white = WhitePixel(dis, screen);
    XColor green_col;
    Colormap colormap;
    char green[] = "#00FF00";

    //variables to move around
    int x_setoff(0);
    int y_setoff(0);
    float scale(1.0);

    // Start window
    #ifdef DEBUG
    cerr << "Creating window" << endl;
    #endif // DEBUG
    win = XCreateSimpleWindow(dis,RootWindow(dis,0),1,1,500,500, 0, black, black);

    XSetStandardProperties(dis,win, "rudisvg", "i!", None, NULL, 0, NULL);

    XSelectInput(dis, win, ExposureMask | KeyPressMask | ButtonPressMask);

    // To close window without error
    Atom delWindow = XInternAtom( dis, "WM_DELETE_WINDOW", 0 );
    XSetWMProtocols(dis , win, &delWindow, 1);
    #ifdef DEBUG
    cerr << "Maping window" << endl;
    #endif // DEBUG
    XMapWindow(dis, win);
    XFlush(dis);
    //sleep(5000);
    #ifdef DEBUG
    cerr << "Setting windows options" << endl;
    #endif // DEBUG

    colormap = DefaultColormap(dis, 0);
    green_gc = XCreateGC(dis, win, 0, 0);


    XParseColor(dis, colormap, green, &green_col);
    XAllocColor(dis, colormap, &green_col);
    XSetForeground(dis, green_gc, green_col.pixel);

    #ifdef DEBUG
    cerr << "Entering loop" << endl;
    #endif // DEBUG
    if (!quiet) {
	cerr << "Zoom in with the + or p key and out with the - or m key, move right and left" << endl;
	cerr << "with arrow keys (Swedish keyboard)." << endl;
    }
    while(1) {
        XNextEvent(dis, &report);
	if (report.type == Expose) {
                display_drawing(dis, win, green_gc, figure,x_setoff,y_setoff,scale);
	}
	else if (report.type == KeyPress) {
		#ifdef DEBUG
		    std::cout << "Key presed: " << XLookupKeysym(&report.xkey,0) << std::endl;
		#endif //DEBUG
                if (XLookupKeysym(&report.xkey,0)==XK_space || XLookupKeysym(&report.xkey,0)==65307 || XLookupKeysym(&report.xkey,0)==113) { // space + esc + q?
                    break;
                }
		else if (XLookupKeysym(&report.xkey,0) == 65364) { // down
		    y_setoff -= 10;
		    XClearWindow(dis, win);
		    display_drawing(dis, win, green_gc, figure,x_setoff,y_setoff,scale);
		}
		else if (XLookupKeysym(&report.xkey,0) == 65362) { // up
		    y_setoff += 10;
		    XClearWindow(dis, win);
		    display_drawing(dis, win, green_gc, figure,x_setoff,y_setoff,scale);
		}
		else if (XLookupKeysym(&report.xkey,0) == 65363) { // right
		    x_setoff -= 10;
		    XClearWindow(dis, win);
		    display_drawing(dis, win, green_gc, figure,x_setoff,y_setoff,scale);
		}
		else if (XLookupKeysym(&report.xkey,0) == 65361) { // left
		    x_setoff += 10;
		    XClearWindow(dis, win);
		    display_drawing(dis, win, green_gc, figure,x_setoff,y_setoff,scale);
		}
		else if (XLookupKeysym(&report.xkey,0) == 43 || XLookupKeysym(&report.xkey,0) == 112) { // +? or p
		    scale *= 1.1;
		    XClearWindow(dis, win);
		    display_drawing(dis, win, green_gc, figure,x_setoff,y_setoff,scale);
		}
		else if (XLookupKeysym(&report.xkey,0) == 45 || XLookupKeysym(&report.xkey,0) == 109) { //-? or m
		    scale *= 0.9;
		    XClearWindow(dis, win);
		    display_drawing(dis, win, green_gc, figure,x_setoff,y_setoff,scale);
		}
	        /*else {
		    std::cout << XLookupKeysym(&report.xkey,0) << std::endl;
		}*/	
	}
	else if (report.type == ClientMessage)
	    break;
    }
    close_x(dis, win, green_gc);
    return(0);
}

bool pars_svg(istream& input, drawing& fig){
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load(input);

    if(result) {
	unsigned int n_objects(0);
        pugi::xml_node objects = doc.child("svg");
	for (pugi::xml_node_iterator object = objects.begin(); object != objects.end(); ++object) {
            if (!strcmp("line", object->name())) {
	        int x1 = atoi(object->attribute("x1").value());
	        int y1 = atoi(object->attribute("y1").value());
	        int x2 = atoi(object->attribute("x2").value());
	        int y2 = atoi(object->attribute("y2").value());
		fig.add_line(x1,y1,x2,y2);
		++n_objects;
		//std::cout << "Added line: " << x1 << ',' << y1 << ' ' << x2 << ',' << y2 << std::endl;
	    }
	    else if (!strcmp("polyline", object->name())) {
                const std::string points = object->attribute("points").value();
                //std::cout << points << endl;
		unsigned int length = points.length();
                std::string value;
		int x1=0;
		int y1=0;
		int x2=0;
		int y2=0;
		bool start = true;
		for (unsigned int i=0; i<=length; ++i) {
                    if ((points[i] == ' ' || i == length) && !value.empty()) {
			//std::cout << "x1,y1: " << x1 << ',' << y1 << "  x2, value: " << x2 << ',' << value << std::endl;
			if (!start) {
                            y2 = atoi(value.c_str());
                            fig.add_line(x1,y1,x2,y2);
			    //std::cout << "Added line: " << x1 << ',' << y1 << ' ' << x2 << ',' << y2 << std::endl;
			    x1 = x2;
			    y1 = y2;
			}
			else {
			    y1 = atoi(value.c_str());
			    start = false;
			}
			value.clear();
		    }
		    else if (points[i] == ',') {
			if (value.empty()) {
                            std::cerr << "Polyline x value parsing error." << std::endl;
			    return false;
			}
                        if (start) {
			    x1 = atoi(value.c_str());
			    //std::cout << "Read first x: " << x1 << std::endl;
			}
			else x2 = atoi(value.c_str());
			value.clear();
		    }
		    else value+=points[i];
		}
		++n_objects;
	    }
	    else if (!strcmp("text", object->name())) {
	        int x = atoi(object->attribute("x").value());
	        int y = atoi(object->attribute("y").value());
		unsigned int font_size = atoi(object->attribute("font-size").value());
		string font = object->attribute("font").value();
		string text_string = object->child_value();
		//std::cout << "Splunch: " << object->child_value() << std::endl;
		fig.add_text(x,y,text_string,font,font_size);
		//std::cout << "Added text string: " << text_string << std::endl;
		++n_objects;
	    }
	    else std::cerr << "Unknown object: " << object->name() << std::endl;
	    if (!(n_objects % 1000)) std::cout << "Added " << n_objects << " objects." << std::endl;
	}
    }
    else return false;
    return true;
}

void display_drawing(Display* dis, Window& win, GC& gc, drawing& fig, const int x_setoff, const int y_setoff, const float scale) {
    fig.init();
    drawing::line line;
    while (fig.more_lines()) {
        line = fig.get_line();
        XDrawLine(dis, win, gc, (line.start.x-x_setoff)*scale, (line.start.y-y_setoff)*scale, (line.end.x-x_setoff)*scale, (line.end.y-y_setoff)*scale);
    }
    drawing::rectangle rectangle;
    while (fig.more_rectangles()){
        rectangle = fig.get_rectangle();
	XDrawRectangle(dis, win, gc, (rectangle.upper_left.x-x_setoff)*scale, (rectangle.upper_left.y-y_setoff)*scale, rectangle.width*scale, rectangle.height*scale);
    }
    drawing::ellipse ellipse;
    while (fig.more_ellipses()) {
        ellipse = fig.get_ellipse();
	XDrawArc(dis, win, gc, (ellipse.upper_left.x-x_setoff)*scale, (ellipse.upper_left.y-y_setoff)*scale, ellipse.width*scale, ellipse.height*scale, ellipse.start_angle, ellipse.end_angle);
    }
    drawing::texttest text;
    // Font test
    Font font(BadName);
    unsigned int font_size(0);
    unsigned int size(0);
    while (fig.more_texts()) {
        text = fig.get_text();
	if (size == 0 || text.font_size != font_size) {
	    font_size = text.font_size;
	    if (font_size >0) size = font_size;
	    else size = 10;
	    size *= scale;
	    bool increase(false);
	    bool end(false);
	    int Nfonts(0);
	    while (font == BadName || font == BadAlloc || !Nfonts) {
		//if (font != BadName) XUnloadFont(dis,font);
		stringstream fontname;
		#ifdef DEBUG
		cerr << font << ' ' << BadName << ' ' << BadAlloc << endl;
		#endif //DEBUG
		fontname << "*x" << size;
		#ifdef DEBUG
		cerr << fontname.str()  << endl;
		#endif //DEBUG
		//char** possible_fonts = XListFonts(dis, fontname.str().c_str(),5,&Nfonts);
		XListFonts(dis, fontname.str().c_str(),5,&Nfonts);
		#ifdef DEBUG
		cerr << "N: " << Nfonts  << endl;
		#endif //DEBUG
		if (Nfonts)
		    font = XLoadFont(dis, fontname.str().c_str());    
		else {
		    if (((size < 5 && !increase) || (size > 30 && increase)) && end) {
			fontname << "*x*";
			font = XLoadFont(dis, fontname.str().c_str());
			break;
		    }
		    else if (size > 30 && increase) { --size; increase=false; end = true; }
		    else if (size < 5 && !increase)  { ++size; increase=true; end = true; }
		    else if (increase) ++size;
		    else --size;
		}
	    }
	    #ifdef DEBUG
	    cerr << "Final font: " << font << ' ' << BadName << endl;
	    #endif //DEBUG
	    #ifdef DEBUG
	    int FontSet = XSetFont(dis,gc,font);
	    cerr << FontSet << endl;
	    #else
	    XSetFont(dis,gc,font);
    	    #endif //DEBUG
	}
    ///////////
	XDrawString(dis, win, gc, (text.start.x-x_setoff)*scale, (text.start.y-y_setoff)*scale, text.characters.c_str(),text.characters.length());
    }
    XUnloadFont(dis,font);
}

void help() {
    cout << "Rudisvg 0.1.0 is a rudimentary svg viewer. It can read and draw lines," << endl;
    cout << "rectangles, ellipsoids, and text. It only display in one color. Zoom in with the" << endl;
    cerr << "+ or p key and out with the - or m key, move right and left with arrow keys" << endl;
    cout << "(Swedish keyboard)." << endl;
    cout << "(c) Martin Ryberg 2016." << endl << endl;
    cout << "Usage:" << endl << "rudisvg < file.svg" << endl << "rudisvg file.svg" << endl << endl;
    cout << "Arguments:" << endl;
    cout << "--file / -f [file]    give svg file name." << endl;
    cout << "--help / -h           print this help." << endl;
    cout << "--verbose / -v        give additional output (to command line)." << endl;

}
