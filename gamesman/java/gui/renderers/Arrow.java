package renderers;

import java.awt.*;

public class Arrow
{
    private Polygon myPoly;

    public Arrow(int x1, int y1, int xe, int ye, int w)
    {
	int[] xlist = new int[7];
	int[] ylist = new int[7];
	int dx = (xe-x1)/10;
	int dy = (ye-y1)/10;
	
	

	while(dx*dx+dy*dy<w*w)
	{
	    dx*=1.1;
	    dy*=1.1;
	}
	 while(dx*dx+dy*dy>w*w)
	{
	    dx*=.9;
	    dy*=.9;
	}

	int x2 = xe-dx*4;
	int y2 = ye-dy*4;

	int curx = x1-dy;
	int cury = y1+dx;
	xlist[0] = curx;
	ylist[0] = cury;
	
	curx = x2-dy;
	cury = y2+dx;
	xlist[1] = curx;
	ylist[1] = cury;
	
	/*Triangle*/

	curx = x2-2*dy;
	cury = y2+2*dx;
	xlist[2] = curx;
	ylist[2] = cury;

	curx = xe;
	cury = ye;
	xlist[3] = curx;
	ylist[3] = cury;

	curx = x2+2*dy;
	cury = y2-2*dx;
	xlist[4] = curx;
	ylist[4] = cury;

	/*End triangle*/

	curx = x2+dy;
	cury = y2-dx;
	xlist[5] = curx;
	ylist[5] = cury;

	curx = x1+dy;
	cury = y1-dx;
	xlist[6] = curx;
	ylist[6] = cury;
	
	myPoly = new Polygon(xlist,ylist,7);
    }

    
    public void fill(Graphics2D g)
    {
	g.fill(myPoly);
    }

    public void draw(Graphics2D g)
    {
	g.draw(myPoly);
    }
 
    public Polygon getPoly()
    {
	return myPoly;
    }
    
}
	
