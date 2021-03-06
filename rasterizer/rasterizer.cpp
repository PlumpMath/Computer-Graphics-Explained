#include <vector>
#include <cstdio>
#include <SDL/SDL.h>
#include <linealg.h>
#include <fixedpoint.h>
#include "myassert.h"

void TriangleSplit(std::vector<Vector4i>& triangle)
{

    unsigned int len = triangle.size();
    for(unsigned int i = 0; i<len; i+=3)
    {
        Vector4i v0 = triangle[i+0];
        Vector4i v1 = triangle[i+1];
        Vector4i v2 = triangle[i+2];
        
        if(v0.y > v1.y)
            std::swap(v0, v1);
        if(v1.y > v2.y)
            std::swap(v1, v2);
        if(v0.y > v1.y)
            std::swap(v0, v1);
        
	if( (v0.y == v1.y) && (v1.y == v2.y) ){
	    /* Ignore degenerate triangles */
            continue;
        } else if( (v0.y == v1.y) || (v1.y == v2.y)){
	    /* don't split if we already have a horisontal edge */
	    triangle.push_back(v0);
	    triangle.push_back(v1);
	    triangle.push_back(v2);
            continue;
	}
        
        /* Split in two triangles */

        Vector4i edge0 = v1 - v0;
        Vector4i edge1 = v2 - v0;
        
	int slopeX, slopeZ, slopeW;

	if(!edge1.y){
	    slopeX = slopeZ = slopeW = 0;
	} else {
	    slopeX =  ((long long)edge1.x << 16) / edge1.y;
	    slopeZ =  ((long long)edge1.z << 16) / edge1.y;
	    slopeW =  ((long long)edge1.w << 16) / edge1.y;
	}

        //new point
	int x_new = v0.x + (((long long)edge0.y * slopeX) >> 16);
	int y_new = v1.y;
	int z_new = v0.z + (((long long)edge0.y * slopeZ) >> 16);
	int w_new = v0.w + (((long long)edge0.y * slopeW) >> 16);

        Vector4i v3(x_new, y_new, z_new, w_new);
	
	triangle.push_back(v1);
	triangle.push_back(v3);
	triangle.push_back(v2);

	triangle.push_back(v0);
	triangle.push_back(v1);
	triangle.push_back(v3);
    }
    triangle.erase(triangle.begin(), triangle.begin() + len);
}

static void TriangleScan(
		  Vector2i start,
		  Vector2i end,
		  int slope0,
		  int slope1,
		  unsigned int* buffer,
		  unsigned int width,
		  unsigned int height,
		  unsigned int color
		  )
{
    int y0 = start.y;
    int y1 = end.y;
    int x0 = start.x;
    int x1 = end.x;

    while(y0 <= y1)
    {
	int PosX = ceilfp(x0);
        int EndX = ceilfp(x1) - (1<<16);
               
	//Vector2i PosTex = tcoord_start;
	//TCoord SlopeTex;
	int xError = PosX - ceilfp(x0);
	int xDelta = x1 - x0;
	int column = y0*width;
                                
	PosX >>= 16;
	EndX >>= 16;

	for(;PosX <= EndX; ++PosX)
	    buffer[PosX + column] = color;
            
	x0 += slope0;
	x1 += slope1;
                    
	++y0;
    }
}


void DrawTriangle(
		  std::vector<Vector4i>& vertexData,
		  unsigned int* buffer,
		  unsigned int width,
		  unsigned int height
		  )
{
    int x0,x1,y0,y1, slope0, slope1;
    Vector4i edge0;
    Vector4i edge1;

    for(unsigned int i=0; i<vertexData.size(); i+=3){
        
        Vector4i& v0 = vertexData[i+0];
        Vector4i& v1 = vertexData[i+1];
        Vector4i& v2 = vertexData[i+2];
    
        /* v0 and v1 is on top */        
        if(v0.y == v1.y){
            if(v1.x < v0.x)
                std::swap(v0, v1);
            
            x0 = v0.x;
            x1 = v1.x;
            y0 = ceilfp(v0.y);
            y1 = ceilfp(v2.y) - (1<<16);
            
	    if(y0 > y1)
		continue;

            edge0 = v2 - v0;
            edge1 = v2 - v1;
            
        }
        else { /* p0 is on top, p1 and p2 is below */
            if(v2.x < v1.x)
                std::swap(v1, v2);
                        
            x0 = v0.x;
            x1 = v0.x;
            y0 = ceilfp(v0.y);
            y1 = ceilfp(v1.y) - (1<<16);
            
            edge0 = v1 - v0;
            edge1 = v2 - v0;
        }
        
        /* slope for leftmost edge */
        if(!edge0.y){
            slope0 = 0;
            //slope_tcoord0 = Vector2f(0,0);
        }
        else{
            slope0 = ((long long)edge0.x<<16) / edge0.y;
        }
        
        /* slope for rightmost edge */
        if(!edge1.y){
            slope1 = 0;
            //slope_tcoord1 = Vector2f(0,0);
        }
        else{
            slope1 = ((long long)edge1.x<<16) / edge1.y;
        }
        
        int yError = y0 - v0.y;
        
        /* correct for offseted start y-coord */
        if(yError)
        {
            x0 += ((long long)slope0*yError) >> 16;
            x1 += ((long long)slope1*yError) >> 16;
        }

        y0 >>= 16;
        y1 >>= 16;
	Vector2i start(x0, y0);
	Vector2i end(x1, y1);
	TriangleScan(start, end, slope0, slope1, buffer, width, height, 0xFFFFFFFF);
    }
}


















































/*

	if(!split)
	    continue;

	int numlines = 1 + v1->y - v0->y;

	start.y += numlines;
	start.x += (numlines * slope1);


	edge0.x = (v2->x - v1->x) * 65536.0f;
	edge0.y = (v2->y - v1->y);
	if(!edge0.y)
            slope0 = 0;
        else
	    slope0 = edge0.x / edge0.y;

 
	end = Vector2i(v1->x * 65536.0f, v2->y);
	
	TriangleScan(start, end, slope1, slope0, buffer, width, height, color);
*/
