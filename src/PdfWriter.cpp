/*
 * PdfWriter
 * Date: Feb-01-2018 
 * Author : Gabriel Renaud gabriel.reno [at sign here ] gmail.com
 *
 */

#include "PdfWriter.h"
jmp_buf env;

#ifdef HPDF_DLL
void  __stdcall
#else
void
#endif
error_handler  (HPDF_STATUS   error_no,
                HPDF_STATUS   detail_no,
                void         *user_data)
{
    printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
                (HPDF_UINT)detail_no);
    longjmp(env, 1);
}

PdfWriter::PdfWriter(const string fname_,
		     double heightChr_ //height of the rectangle set by the user
){
     fname     = fname_;
     heightChr = heightChr_;
     
    //string fname      = string(argv[indexOflastOpt]);
     string page_title = "";

     //gray = {0.6,0.6,0.6};
     // gray[0]  = 0.6;
     // gray[1]  = 0.6;
     // gray[2]  = 0.6;
     // cerr<<"test gray"<<endl;
     
	 
     pdf = HPDF_New (error_handler, NULL);
     if (!pdf) {
	 cerr<<"error: cannot create PdfDoc object for file="<<fname<<endl;
	 exit(1);
     }

     if(setjmp(env)){
	 HPDF_Free (pdf);
	 cerr<<"error: jump failed file="<<fname<<endl;
	 exit(1);
     }
     font = HPDF_GetFont (pdf, "Helvetica", NULL);

     /* add a new page object. */
     page = HPDF_AddPage (pdf);





     // /* print the lines of the page. */
     //HPDF_Page_SetLineWidth (page, 1);
     // HPDF_Page_Rectangle (page, 50, 50, HPDF_Page_GetWidth(page) - 100,
     //             HPDF_Page_GetHeight (page) - 110);
     // HPDF_Page_Stroke (page);

     /* print the title of the page (with positioning center). */
     HPDF_Page_SetFontAndSize (page, font, 24);
     tw = HPDF_Page_TextWidth (page, page_title.c_str());
     HPDF_Page_BeginText (page);
     HPDF_Page_MoveTextPos (page, (HPDF_Page_GetWidth(page) - tw) / 2,
			    HPDF_Page_GetHeight (page) - 50);
     //HPDF_Page_ShowText (page, page_title.c_str());
     HPDF_Page_EndText (page);
     HPDF_Page_SetFontAndSize (page, font, 10);
     //cerr<<"Creating a page in file  "<<fname_<<"w="<<HPDF_Page_GetWidth(page)<<" h="<<HPDF_Page_GetHeight (page)<<endl;

  
}

PdfWriter::~PdfWriter(){
    
     /* save the document to a file */
    cerr<<"Saving PDF file "<<fname<<endl;
    HPDF_SaveToFile (pdf, fname.c_str());

     /* clean up */
     HPDF_Free (pdf);

}

void PdfWriter::draw_rect (HPDF_Page     page,
			   double        x,
			   double        y,
			   double length,
			   const char   *label){    
    
    //cerr<<"draw_rect "<<x<<","<<y<<" l = "<<length<<" label="<<label<<" "<<GRAY<<endl;
    //text pos
    HPDF_Page_BeginText (page);
    HPDF_Page_MoveTextPos (page, x, y - 10);
    HPDF_Page_ShowText (page, label);
    HPDF_Page_EndText (page);
    
    //rectangle
    HPDF_Page_SetRGBStroke  (page,
    			     GRAY,// GRAY,// gray[0],
    			     GRAY,// GRAY,// gray[1],
    			     GRAY// GRAY// gray[2]
    );
    //cerr<<"set1 "<<endl;
    HPDF_Page_Rectangle(page,
			x,             //x
			y - (heightChr+heightLabel),  //y  offset minus the heightLabel and height
			length,  //width
			heightChr);     //height
    ///cerr<<"set2 "<<endl;
     // HPDF_Page_SetRGBStroke  (page,
     // 			      0,// 0.0,
     // 			      0,// 0.0,
     // 			      0);// 0.0);
    //cerr<<"done"<<endl;
    // HPDF_Page_Fill (page);
}


void PdfWriter::draw_Simplerect (HPDF_Page     page,
				 double        x,
				 double        y,
				 double length){    
    // cout<<"x "<<x<<endl;
    // cout<<"y "<<y<<endl;

    length=max(length,0.4);
    // cout<<"l "<<length<<endl;
    // HPDF_Page_BeginText (page);
    // //text pos
    // HPDF_Page_MoveTextPos (page, x, y - 10);
    // HPDF_Page_ShowText (page, label);
    // HPDF_Page_EndText (page);
    //rectangle

    HPDF_Page_Rectangle(page, x, y - 30, length, 15);
}

inline void PdfWriter::addRange(HPDF_Page & page,double begin,double end, const chrScreenInfo & chrInfToUse ){
    draw_Simplerect (page, 
		     10+ chrInfToUse.lengthScreen*(begin/chrInfToUse.length ),
		     chrInfToUse.y,
		     chrInfToUse.lengthScreen * ((end-begin)/chrInfToUse.length) );			     
    // cout<<line<<endl;
    HPDF_Page_Fill (page);
}

inline void PdfWriter::addRangeCov(HPDF_Page & page,double begin,double end, const chrScreenInfo & chrInfToUse, double covFrac , int indexofinputF){
    // cout<<"1 "<<(covFrac)<<endl;
    // cout<<"2 " <<(1.0-covFrac)<<endl;
    //    covFrac=double(rand())/double(RAND_MAX);



    if(indexofinputF==1){
	HPDF_Page_SetRGBStroke (page, 1.00,        1.0-covFrac,  1.0-covFrac);
	HPDF_Page_SetRGBFill   (page, 1.00,        1.0-covFrac,  1.0-covFrac);
    }else if(indexofinputF==2){
	HPDF_Page_SetRGBStroke (page, 1.0-covFrac, 1.00,         1.0-covFrac);
	HPDF_Page_SetRGBFill   (page, 1.0-covFrac, 1.00,         1.0-covFrac);
    }else if(indexofinputF==3){
	HPDF_Page_SetRGBStroke (page, 1.0-covFrac, 1.0-covFrac,  1.00);
	HPDF_Page_SetRGBFill   (page, 1.0-covFrac, 1.0-covFrac,  1.00);
    }else{
	cerr<<"Color not defined for this file"<<endl;
	exit(1);
    }

    draw_Simplerect (page, 
		     10+ chrInfToUse.lengthScreen*(begin/chrInfToUse.length ),
		     chrInfToUse.y,
		     chrInfToUse.lengthScreen * ((end-begin)/chrInfToUse.length) );			     
    // cout<<line<<endl;
    HPDF_Page_Fill (page);
}


int PdfWriter::drawFrame(const string & fastaIndex,
			 const double   windowSizeForHest){
    
     string line;
     igzstream myFaidxFile;
     // bool oneChr=false;
     // string oneChrName="";

     myFaidxFile.open(fastaIndex.c_str(), ios::in);


     vector<chrinfo> chrFound;
     unsigned int  genomeLength=0;
     unsigned int  maxLengthFound=0;
     if (myFaidxFile.good()){
	 while ( getline (myFaidxFile,line)){
	     chrinfo toadd;
	     vector<string> fields = allTokens(line,'\t');

	     toadd.name         =fields[0];
	     toadd.startIndexChr=genomeLength+1;
	     toadd.length       =destringify<unsigned int>(fields[1]);
	     if(toadd.length> maxLengthFound){
		 maxLengthFound = toadd.length;
	     }
	     toadd.endIndexChr  =genomeLength+toadd.length;

	     chrFound.push_back(toadd);

	 }
	 myFaidxFile.close();
     }else{
	 cerr << "Unable to open fasta index file "<<fastaIndex<<endl;
	 return 1;
     }


     
     //bool found=false;
     // if(oneChr){
     // 	 for(unsigned int i=0;
     // 	     i<chrFound.size();
     // 	     i++){
     // 	     if(chrFound[i].name == oneChrName){
     // 		 found=true;
     // 		 maxLengthFound=chrFound[i].length;
     // 	     }
     // 	 }

     // 	 if(!found){
     // 	     cerr<<"Chromosome you entered "<<oneChrName<<" was not found"<<endl;
     // 	     return 1;
     // 	 }
     // }


     double sizeToUse=HPDF_Page_GetHeight(page)/double(2.0*chrFound.size());


     double widthScreen= (HPDF_Page_GetWidth(page)-10.0);    
     for(unsigned int i=0;
	 i<chrFound.size();
	 i++){

	 double yOffset=sizeToUse*double(i*2);
	 //cerr<<"i1 ="<<i<<"\t"<<yOffset<<endl;
	 // if(oneChr){
	 //     if(chrFound[i].name != oneChrName)
	 // 	 continue;
	 //     yOffset=0;
	 // }
	
	 draw_rect (page, 
		    xmargin ,                            //x
		    HPDF_Page_GetHeight(page) - yOffset, //y
		    widthScreen  * (double(chrFound[i].length)/double(maxLengthFound)),     //length
		    //heightChr,//height
		    chrFound[i].name.c_str());
	 //cerr<<"i2 ="<<i<<"\t"<<yOffset<<endl;	 
	 name2chrScreenInfo[ chrFound[i].name.c_str() ].y            = HPDF_Page_GetHeight(page) -yOffset; //y offset
	 name2chrScreenInfo[ chrFound[i].name.c_str() ].length       = double(chrFound[i].length);   //actual length in bases 
	 name2chrScreenInfo[ chrFound[i].name.c_str() ].lengthScreen = ( (HPDF_Page_GetWidth(page)-10.0)  * (double(chrFound[i].length)/double(maxLengthFound)) );     //length on screen in pixel

	 //cerr<<"i3 ="<<i<<"\t"<<yOffset<<endl;



	 //top corner minus height label + height chr drawing

	 HPDF_Page_Stroke (page);
	 //cerr<<
     }

     //cerr<<"test "<<endl;
     

     HPDF_Page_GSave (page);
     gstate = HPDF_CreateExtGState (pdf);
     HPDF_ExtGState_SetAlphaFill (gstate, alpha);
     HPDF_Page_SetExtGState (page, gstate);


     //write out x labels
     for(unsigned int i=0;
	 i<chrFound.size();
	 i++){
	 unsigned int mark = 100000000;
	 while(mark>=1){
	     //cerr<<"mark "<<mark<<endl;
	     if( name2chrScreenInfo[ chrFound[i].name.c_str() ].length > mark){
		 name2chrScreenInfo[ chrFound[i].name.c_str() ].markings = mark;
		 break;
	     }else{
		 mark           = mark / 10;
	     }
	 }
	 //cerr<<"mark "<<mark<<endl;
	 //draw markings

	 for(unsigned int m=0;m<name2chrScreenInfo[ chrFound[i].name.c_str() ].length;m+=mark ){
	     //name2chrScreenInfo[ chrFound[i].name.c_str() ].y - (heightChr+heightLabel)
	     double xfraction = ( m  /(name2chrScreenInfo[ chrFound[i].name.c_str() ].length+windowSizeForHest));
	     double x = xmargin +   xfraction*name2chrScreenInfo[ chrFound[i].name.c_str() ].lengthScreen ;
	     double y1 = name2chrScreenInfo[ chrFound[i].name.c_str() ].y - (heightChr+heightLabel);
	     double y2 = name2chrScreenInfo[ chrFound[i].name.c_str() ].y - (heightChr+heightLabel)-heightLabel/5.0;//1/5 of the height label
	     
	     //cerr<<"x "<<x<<" m "<<m<<" "<<(name2chrScreenInfo[ chrFound[i].name.c_str() ].y - (heightChr+heightLabel))<<endl;

	     // HPDF_Page_SetRGBStroke  (page,
	     // 			      gray[0],
	     // 			      gray[1],
	     // 			      gray[2]);
	     
	     drawVerticalLine( x,                                                                                                             // x offset
	      		       y1,  // baseline
	     		       y2,
	     		       GRAY,// gray[0],
	     		       GRAY,// gray[1],
	     		       GRAY,// gray[2],
	     		       1.0//widthtouse/3.0
	     ); // baseline

	     //text pos
	     string labl ;
	     if(m==0){
		 labl = "0";
	     }else{
		 if(m > 1000000){
		     labl     = stringify(m/1000000)+"Mb";
		 }else{
		     if(m > 1000){
			 labl = stringify(m/1000)   +"kb";
		     }else{
			 labl = stringify(m)        + "b";
		     }
		 }
		 //labl = "0";
	     }
	     
	     //cerr<<"labl "<<labl<<endl;
	     double fontsize =5;
	     HPDF_Page_SetFontAndSize (page, font, 5);

	     HPDF_Page_BeginText   (page);
	     HPDF_Page_MoveTextPos (page, x - double(labl.size())/2.0, y2-fontsize);
	     HPDF_Page_ShowText    (page, labl.c_str() );
	     HPDF_Page_EndText     (page);
	     
	 }
     }

    
     return 0;
}

int PdfWriter::drawYLabels(const long double minHFoundPlotting,
			   const long double maxHFoundPlotting,
			   const bool scientific){
    string formattouse;
    if(scientific){
	formattouse="%.0e\n";
    }else{
	formattouse="%.1f\n";
    }
    
    //write out y labels
    for(map<string , chrScreenInfo>::iterator it = name2chrScreenInfo.begin(); it != name2chrScreenInfo.end(); ++it) {

	
	// for(unsigned int i=0;
	//  	 i<chrFound.size();
	//  	 i++){
	
	// double xfraction = ( m  /(name2chrScreenInfo[ chrFound[i].name.c_str() ].length+windowSizeForHest));
	// double x = xmargin +   xfraction*name2chrScreenInfo[ chrFound[i].name.c_str() ].lengthScreen ;
	double x  = xmargin;
	double y1 = name2chrScreenInfo[ it->first ].y - (          heightLabel); //top
	double y2 = name2chrScreenInfo[ it->first ].y - (heightChr+heightLabel); //bottom

	//cerr<<"drawYLabels "<<x<<" "<<y1<<" "<<y2<<endl;
	double fontsize =2;
	stringstream stream;
	string labl;
	
	//labl=stringify(minHFoundPlotting);
	stream<<minHFoundPlotting;
	labl=stream.str();
	//cout<<"labl min "<<labl<<endl;
	HPDF_Page_SetFontAndSize (page, font, fontsize);

	
	HPDF_Page_BeginText   (page);
	HPDF_Page_MoveTextPos (page, x - double(labl.size())*fontsize*2, y2-fontsize/2.0); // x - double(labl.size())/2.
	HPDF_Page_ShowText    (page, labl.c_str() );
	HPDF_Page_EndText     (page);

	drawHorizontalLine((x - x/4.0), //0,//x/2.0  ,
			   x,
			   y1,
			   GRAY,
			   GRAY,
			   GRAY,
			   1.0       );

	//labl=stringify(maxHFoundPlotting);
	char buf[64];
	
	sprintf(buf,formattouse.c_str(), double(maxHFoundPlotting));
	
	labl = string(buf);
	//cout<<"labl max "<<labl<<endl;
	HPDF_Page_SetFontAndSize (page, font, fontsize);
	
	HPDF_Page_BeginText   (page);
	HPDF_Page_MoveTextPos (page, x - double(labl.size())*1.5, y1-fontsize/4.0);
	HPDF_Page_ShowText    (page, labl.c_str() );
	HPDF_Page_EndText     (page);
	
	
	drawHorizontalLine((x - x/4.0), //0,//x/2.0  ,
			   x,
			   y2,
			   GRAY,
			   GRAY,
			   GRAY,
			   1.0       );



	double factor=double(1)/double(3.0);
	double y3 = name2chrScreenInfo[ it->first ].y - (heightChr*(1-factor)+heightLabel); //bottom

	
	sprintf(buf,formattouse.c_str(), double(maxHFoundPlotting*factor));

	labl = string(buf);
	//cout<<"labl max "<<labl<<endl;
	HPDF_Page_SetFontAndSize (page, font, fontsize);
	
	HPDF_Page_BeginText   (page);
	HPDF_Page_MoveTextPos (page, x - double(labl.size())*1.5, y3-fontsize/4.0);
	HPDF_Page_ShowText    (page, labl.c_str() );
	HPDF_Page_EndText     (page);
	
	
	drawHorizontalLine((x - x/4.0), //0,//x/2.0  ,
			   x,
			   y3,
			   GRAY,
			   GRAY,
			   GRAY,
			   1.0       );


	factor=double(2)/double(3.0);
	double y4 = name2chrScreenInfo[ it->first ].y - (heightChr*(1-factor)+heightLabel); //bottom

	sprintf(buf,formattouse.c_str(), double(maxHFoundPlotting*factor));
	
	labl = string(buf);
	//cout<<"labl max "<<labl<<endl;
	HPDF_Page_SetFontAndSize (page, font, fontsize);
	
	HPDF_Page_BeginText   (page);
	HPDF_Page_MoveTextPos (page, x - double(labl.size())*1.5, y4-fontsize/4.0);
	HPDF_Page_ShowText    (page, labl.c_str() );
	HPDF_Page_EndText     (page);
	
	
	drawHorizontalLine((x - x/4.0), //0,//x/2.0  ,
			   x,
			   y4,
			   GRAY,
			   GRAY,
			   GRAY,
			   1.0       );


	

	
	// cerr<<"x "<<x<<" m "<<m<<" "<<(name2chrScreenInfo[ chrFound[i].name.c_str() ].y - (heightChr+heightLabel))<<endl;	 
	// // HPDF_Page_SetRGBStroke  (page,
	// // 			      gray[0],
	// // 			      gray[1],
	// // 			      gray[2]);	
	// drawVerticalLine( x,                                                                                                             // x offset
	//  		       y1,  // baseline
	// 		       y2,
	// 		       GRAY,// gray[0],
	// 		       GRAY,// gray[1],
	// 		       GRAY,// gray[2],
	// 		       1.0//widthtouse/3.0
	
    }
    return 0;
}
   


int PdfWriter::drawVerticalLine(const double x,
				const double y1,
				const double y2,
				double r,
				double g,
				double b,
				double w, //width
				bool   dash
){
    // HPDF_Page_BeginText (page);
    // HPDF_Page_MoveTextPos (page, x, y - 10);
    // HPDF_Page_ShowText (page, label);
    // HPDF_Page_EndText (page);
    //cerr<<"drawHorizontalLine "<<x<<" "<<y1<<" "<<y2<<endl;
    if(dash){
	HPDF_Page_SetDash (page, DASH_MODE1, 1, 1);//set line dash
    }

    HPDF_Page_SetRGBStroke (page, r, g, b);//set color
    HPDF_Page_SetLineWidth (page, w); //set line width
    
    HPDF_Page_MoveTo (page, x,   y1);
    HPDF_Page_LineTo (page, x,   y2);
    HPDF_Page_Stroke (page);
    
    HPDF_Page_SetRGBStroke (page, 0, 0, 0); //unset color
    HPDF_Page_SetLineWidth (page, 1.0); //unset line width

    HPDF_Page_SetDash (page, NULL, 0, 0); //unset line dash
    
    return 0;
}


int PdfWriter::drawHEst(const GenomicRange  cr,         // genomic range to plot
			const long double   h_,          // value of h
			const long double   hlow_,       // lower  conf. int. for h
			const long double   hhigh_,      // higher conf. int. for h
			const double        hLimLow,    // lower  limit for the h plot 
			const double        hLimHigh,
			const double        windowSizeForHest){  // higher limit for the h plot 
    // HPDF_Page_BeginText (page);
    // HPDF_Page_MoveTextPos (page, x, y - 10);
    // HPDF_Page_ShowText (page, label);
    // HPDF_Page_EndText (page);
    long double   h     = h_;
    long double   hlow  = hlow_;
    long double   hhigh = hhigh_;

    //if the hLimHigh was bound to avoid large confidence interval
    if(hhigh > hLimHigh){
	hhigh  = hLimHigh;
    }
    //cerr<<"drawHEst "<<cr<<" "<<h<<" "<<hlow<<" "<<hhigh<<" hlims "<<hLimLow<<" "<<hLimHigh<<	endl;

    if( name2chrScreenInfo.find( cr.getChrName() ) == name2chrScreenInfo.end() ){
	cerr<<"PdfWriter chromosome not found: "<<cr.getChrName()<<endl;
	return 1;	
    }

    double xfraction = ( ( (cr.getEndCoord()+cr.getStartCoord())/2.0 )/(name2chrScreenInfo[ cr.getChrName() ].length+windowSizeForHest));
    
    //cerr<<"drawHEst2 "<<cr.getEndCoord()<<" "<<cr.getStartCoord()<<" "<<name2chrScreenInfo[ cr.getChrName() ].y<<" l="<<name2chrScreenInfo[ cr.getChrName() ].length<<" "<<name2chrScreenInfo[ cr.getChrName() ].lengthScreen<<" "<<(( (h-hLimLow)/hLimHigh) ) <<" xf="<<xfraction<<endl;

    double x = xmargin +   xfraction*name2chrScreenInfo[ cr.getChrName() ].lengthScreen ;
	
    // drawHorizontalLine( x,//x offset
    // 			name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel), //baseline
    // 			name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (h-hLimLow)/hLimHigh) );//baseline
    //cerr<<"PX "<<(name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (hhigh-hLimLow)/hLimHigh))<<endl;
    // upper estimate

    // drawHorizontalLine( x,                                                                                                             // x offset
    //  			name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (h    -hLimLow)/hLimHigh) ,  // baseline
    //  			name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (hhigh-hLimLow)/hLimHigh),
    // 			0,
    // 			0,
    // 			0.5,
    // 			0.0
    // ); // baseline
    //cerr<<name2chrScreenInfo[ cr.getChrName() ].le
    double widthtouse = name2chrScreenInfo[ cr.getChrName() ].lengthScreen/(name2chrScreenInfo[ cr.getChrName() ].length/windowSizeForHest);


    drawVerticalLine( x,                                                                                                             // x offset
		      name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (h    -hLimLow)/hLimHigh) ,  // baseline
		      name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (hhigh-hLimLow)/hLimHigh),
		      0.9,
		      0,
		      0,
		      0.0//widthtouse/3.0
    ); // baseline

    //cerr<<"val "<<name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (h    -hLimLow)/hLimHigh) <<endl;
    

    drawHorizontalLine( x-widthtouse/2.0,                                                                                                             // x offset
			x+widthtouse/2.0,                                                                                                             // x offset			
			name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (h    -hLimLow)/hLimHigh),                   // baseline
			0,
			0,
			0,
			0.2);
			


    // lower estimate
    drawVerticalLine( x,                                                                                                             // x offset
		      name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (hlow -hLimLow)/hLimHigh) ,  // baseline
		      name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (h    -hLimLow)/hLimHigh) ,
		      0.9,
		      0.0,
		      0.0,
		      0.0//widthtouse/3.0
    ); // baseline

    // drawHorizontalLine( x,                                                                                                             // x offset
    //  			name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (h    -hLimLow)/hLimHigh)-1 ,  // baseline
    //  			name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (h    -hLimLow)/hLimHigh)+1,
    // 			0,
    // 			0,
    // 			0.9,
    // 			2.0
    // ); // baseline

    //  			name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (h    -hLimLow)/hLimHigh)-1 ,  // baseline
    //  			name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (h    -hLimLow)/hLimHigh)+1,
    // 			0,
    // 			0,
    // 			0.9,
    // 			2.0
    // ); 
    // drawVerticalLine( x,                                                                                                             // x offset
    // 		      name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (h    -hLimLow)/hLimHigh),  // baseline
    // 		      name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (h    -hLimLow)/hLimHigh),
    // 		      0.9,
    // 		      0,
    // 		      0,
    // 		      widthtouse
    // ); // baseline

    //name2chrScreenInfo[ chrFound[i].name.c_str() ].length       = double(chrFound[i].length);   //length
    // HPDF_Page_MoveTo (page, x,   y1);
    // HPDF_Page_LineTo (page, x,   y2);
    // HPDF_Page_Stroke (page);
    
    return 0;
}




int PdfWriter::drawHMM(const GenomicRange  cr,         // genomic range to plot
		       const long double   h_,          // value of h
		       const long double   hlow_,       // lower  conf. int. for h
		       const long double   hhigh_,      // higher conf. int. for h
		       const double        hLimLow,    // lower  limit for the h plot 
		       const double        hLimHigh,
		       const double        windowSizeForHest){  // higher limit for the h plot 
    long double   h     = h_;
    //long double   hlow  = hlow_;
    long double   hhigh = hhigh_;

    //if the hLimHigh was bound to avoid large confidence interval
    if(hhigh > hLimHigh){
	hhigh  = hLimHigh;
    }
    //cerr<<"drawHMM "<<cr<<" "<<h<<" "<<hlow<<" "<<hhigh<<" hlims "<<hLimLow<<" "<<hLimHigh<<	endl;

    if( name2chrScreenInfo.find( cr.getChrName() ) == name2chrScreenInfo.end() ){
	cerr<<"PdfWriter chromosome not found: "<<cr.getChrName()<<endl;
	return 1;	
    }

    double xfraction = ( ( (cr.getEndCoord()+cr.getStartCoord())/2.0 )/(name2chrScreenInfo[ cr.getChrName() ].length+windowSizeForHest));
    
    //cerr<<"drawHEst2 "<<cr.getEndCoord()<<" "<<cr.getStartCoord()<<" "<<name2chrScreenInfo[ cr.getChrName() ].y<<" l="<<name2chrScreenInfo[ cr.getChrName() ].length<<" "<<name2chrScreenInfo[ cr.getChrName() ].lengthScreen<<" "<<(( (h-hLimLow)/hLimHigh) ) <<" xf="<<xfraction<<endl;

    double x = xmargin +   xfraction*name2chrScreenInfo[ cr.getChrName() ].lengthScreen ;
	
    double widthtouse = name2chrScreenInfo[ cr.getChrName() ].lengthScreen/(name2chrScreenInfo[ cr.getChrName() ].length/windowSizeForHest);




    drawHorizontalLine( x-widthtouse/2.0,                                                                                                             // x offset
			x+widthtouse/2.0,                                                                                                             // x offset			
			name2chrScreenInfo[ cr.getChrName() ].y - (heightChr+heightLabel) + heightChr*  ( (h    -hLimLow)/hLimHigh),                   // baseline
			0,
			1.0,
			0,
			1.0);
			

    
    return 0;
}


int PdfWriter::drawGlobalHEst(//string chrname,
			      const long double   h,
			      const long double   hlow,
			      const long double   hhigh,
			      const double hLimLow,
			      const double hLimHigh){
    //cerr<<"drawGlobalHEst "<<h<<" "<<hlow<<" "<<hhigh<<endl;
    // if( name2chrScreenInfo.find( chrname ) == name2chrScreenInfo.end() ){
    // 	cerr<<"PdfWriter chromosome not found: "<<chrname<<endl;
    // 	return 1;	
    // }
    
    vector<string> allnames;
    for(map<string , chrScreenInfo>::iterator it = name2chrScreenInfo.begin(); it != name2chrScreenInfo.end(); ++it) {
	// //low
	// drawVerticalLine(xmargin,
	// 		 name2chrScreenInfo[ it->first ].lengthScreen,
	// 		 name2chrScreenInfo[ it->first ].y - (heightChr+heightLabel) + heightChr*  ( (hlow  -hLimLow)/hLimHigh) );//baseline
	//middle
	drawHorizontalLine(xmargin,
			   xmargin+name2chrScreenInfo[ it->first ].lengthScreen,
			   name2chrScreenInfo[ it->first ].y - (heightChr+heightLabel) + heightChr*  ( (h     -hLimLow)/hLimHigh),
			   0.0,
			   0.0,
			   0.7,
			   0.3,
			   true

	);//baseline
	// //high
	// drawVerticalLine(xmargin,
	// 		 name2chrScreenInfo[ it->first ].lengthScreen,
	// 		 name2chrScreenInfo[ it->first ].y - (heightChr+heightLabel) + heightChr*  ( (hhigh -hLimLow)/hLimHigh) );//baseline
	
    }
    
    return 0;
}

int PdfWriter::drawHorizontalLine(const double x1,const double x2,const double y,
				  double r,
				  double g,
				  double b,
				  double w,
				  bool   dash
){

    if(dash){
	HPDF_Page_SetDash (page, DASH_MODE1, 1, 1);//set line dash
    }
    HPDF_Page_SetRGBStroke (page, r, g, b);//set color
    HPDF_Page_SetLineWidth (page, w); //set line width

    HPDF_Page_MoveTo (page, x1,   y);
    HPDF_Page_LineTo (page, x2,   y);
    HPDF_Page_Stroke (page);

    //if(dash){
    HPDF_Page_SetDash (page, NULL, 0, 0); //unset line dash
    //}
    HPDF_Page_SetRGBStroke (page, 0.0, 0.0, 0.0 );//unset color
    HPDF_Page_SetLineWidth (page, 1.0); //unset line width

    return 0;
}
