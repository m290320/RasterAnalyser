#ifndef RASTERIMAGE_H
#define RASTERIMAGE_H

#include <map>
#include <windows.h>
#include <Rcpp.h>


using namespace Rcpp;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////

#define GRIDSIZEDEFAULT 10
#define TILESIZE 5000
#define INDEXSIZE 1000
#define DEFAULTINDEX 25 // Number of index squares that are automatically indexed based on bounding rect only

const int BITSBYTE = 8;

///////////////////////////////////////////////////////////////////////////////

struct Point
{
   long x;
   long y;

   Point(int x1, int y1) {x = x1; y = y1;}
};

///////////////////////////////////////////////////////////////////////////////

struct Rect : public RECT
{
  Rect() {left = -1; right=-1; top=-1; bottom=-1;}
  Rect(int left1, int top1, int width, int height) {left = left1; top = top1; right=left+width; bottom=top-height;} //!!Check

  bool isNull() {return left == -1 && right == -1 && top == -1 && bottom == -1;}
  Rect intersected(Rect &r2) {Rect rect; if (IntersectRect(&rect, this, &r2))return rect; else return Rect();};
  Rect united(Rect &r2) {if (!isNull() && !r2.isNull()){Rect rect; UnionRect(&rect, this, &r2); return rect;} else {return Rect();}}
};

//////////////////////////////////////////////////////////////////////////////////////////////

typedef Point TileIndex;

inline bool operator<(const TileIndex &t1, const TileIndex t2)
{
  return t1.x < t2.x || (t1.x== t2.x && t1.y < t2.y);
}

//////////////////////////////////////////////////////////////////////////////////////////////

class RasterBitmap
{
public:
   RasterBitmap();
   ~RasterBitmap();

   bool Initialise(int nLeft, int nTop);
   double GetExtent(Rect extent);
   bool DrawPolygon(NumericVector &arrayX, NumericVector &arrayY, NumericVector &aLengths, int left, int top);
   bool Intersect(RasterBitmap *pRasterBitmap, Rect extent);
   bool IsEmpty() {return m_hbitmap == NULL || m_pData == NULL;}
   bool Clear();

   double GetExtent(Rect extent, int jMin, int jMax);

   Rect GetRect();
   void* GetData() {return m_pData;}
   void SetRasterImage(class RasterImage *p) {m_pRasterImage = p;}
   int GetResolution();

protected:

   void *m_pData;
   int m_nWidth;
   int m_nHeight;
   int m_nWidthBytes;
   int m_nLeft;
   int m_nTop;
   int m_nGridSize;

   HBITMAP m_hbitmap, m_hbitmapold;
   HDC m_hdc;
   HBRUSH m_hbrushold;
   HPEN m_hpenold;

   class RasterImage *m_pRasterImage;
};

///////////////////////////////////////////////////////////////////////////////

class RasterImage
{

friend class RasterBitmap;

public:
  RasterImage(int nResolution=GRIDSIZEDEFAULT);
  ~RasterImage();

  bool Initialise();
  void Clear();
  double GetExtent(Rect rectIntersect = Rect());
  bool DrawPolygon(NumericVector &arrayX, NumericVector &arrayY, NumericVector &aLengths, Rect &extent,  RasterImage* pIntersectImage = NULL);
  bool Intersect(RasterImage *pRasterImage, Rect rectIntersect = Rect());
  bool IntersectsTiles(Rect rext);

  Rect GetTotalExtent() {return m_rectExtent;}

protected:

 Rect GetPolygonExtent(NumericVector &arrayX, NumericVector &arrayY, NumericVector &aLengths);
 void UpdateTotalExtent(Rect extent);

protected:

  // Store bitmaps in a lookup based on top left position
  map<TileIndex, RasterBitmap*> m_aRaster;

public:
  HBRUSH GetBrush() {return m_hbrush;}
  HPEN GetPen() {return m_hpen;}
protected:
  HBRUSH m_hbrush;
  HPEN m_hpen;

  Rect m_rectExtent; // Extent of polygon
  int m_nResolution;
};


#endif // RASTERIMAGE_H
