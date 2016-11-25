
#include "rasteranalyser.h"
#include <stdio.h>

// This is a simple example of exporting a C++ function to R. You can
// source this function into an R session using the Rcpp::sourceCpp
// function (or via the Source button on the editor toolbar). Learn
// more about Rcpp at:
//
//   http://www.rcpp.org/
//   http://adv-r.had.co.nz/Rcpp.html
//   http://gallery.rcpp.org/
//

///////////////////////////////////////////////////////////////////////////////
//
// [[Rcpp::export]]

XPtr<int> Initialise(int nResolution = 10)
{
   RasterImage *pRasterImage = new RasterImage(nResolution);

return (XPtr<int>)(int*)pRasterImage;
}

///////////////////////////////////////////////////////////////////////////////
//
// [[Rcpp::export]]

XPtr<int> Destroy(XPtr<int> pRasterImage)
{
  if (((RasterImage*)(int*)pRasterImage) != NULL)
  {
     delete (RasterImage*)(int*)pRasterImage;
  }
  return (XPtr<int>)(int*)NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
// [[Rcpp::export]]

bool DrawPolygon(XPtr<int> pRasterImage, NumericVector arrayX, NumericVector arrayY, NumericVector aLengths)
{
   Rect extent;

   // TODO add optional argument second RasterImage
   return ((RasterImage*)(int*)pRasterImage)->DrawPolygon(arrayX, arrayX, aLengths, extent);
}

///////////////////////////////////////////////////////////////////////////////
//
// [[Rcpp::export]]

bool Intersect(XPtr<int> pRasterImage1, XPtr<int> pRasterImage2, NumericVector rectIntersect)
{
   Rect rect;

   if (!rectIntersect.isNULL())
   {
      rect = Rect(rectIntersect[1], rectIntersect[2], rectIntersect[3], rectIntersect[4]);
   }

   return ((RasterImage*)(int*)pRasterImage1)->Intersect((RasterImage*)(int*)pRasterImage2, rect);
}

///////////////////////////////////////////////////////////////////////////////
//
// [[Rcpp::export]]

double GetExtent(XPtr<int> pRasterImage, NumericVector rectIntersect)
{
   Rect rect;

   if (!rectIntersect.isNULL())
   {
      rect = Rect(rectIntersect[1], rectIntersect[2], rectIntersect[3], rectIntersect[4]);
   }

   return ((RasterImage*)(int*)pRasterImage)->GetExtent(rect);
}

///////////////////////////////////////////////////////////////////////////////
//
// [[Rcpp::export]]

void Clear(XPtr<int> pRasterImage)
{
   if (((int*)pRasterImage) != NULL)
   {
      ((RasterImage*)(int*)pRasterImage)->Clear();
   }
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Non-exported functions
//

RasterImage::RasterImage(int nResolution)
{
   m_nResolution = nResolution;
   m_hbrush = CreateSolidBrush(RGB(255,255,255));
   m_hpen = CreatePen(PS_NULL, 0, RGB(255,255,255));
}

///////////////////////////////////////////////////////////////////////////////

RasterImage::~RasterImage()
{
   if (m_hbrush != NULL) DeleteObject(m_hbrush);
   if (m_hpen != NULL) DeleteObject(m_hpen);

   Clear();
}

///////////////////////////////////////////////////////////////////////////////////////

void RasterImage::Clear()
{
   map<TileIndex, RasterBitmap*>::iterator iBitmap;

   for (iBitmap = m_aRaster.begin(); iBitmap != m_aRaster.end(); iBitmap++)
   {
      if (m_aRaster[iBitmap->first] != NULL)
      {
         iBitmap->second->Clear(); // Not called in destructor, so needs explicit call
         delete iBitmap->second;
      }
   };
   m_aRaster.clear();

   m_rectExtent = Rect();
}

////////////////////////////////////////////////////////////////////////////////
//
// Returns extent in hectares

double RasterImage::GetExtent(Rect rectIntersect)
{
   double dArea = 0;

   Rect rect = m_rectExtent;
   if (!rectIntersect.isNull()) rect = m_rectExtent.intersected(rectIntersect);

   map<TileIndex, RasterBitmap*>::iterator iBitmap;

   for (iBitmap = m_aRaster.begin(); iBitmap != m_aRaster.end(); iBitmap++)
   {
      dArea += iBitmap->second->GetExtent(rect);
   };

   return dArea;
};

/////////////////////////////////////////////////////////////////////////////////////////

bool RasterImage::DrawPolygon(NumericVector &arrayX, NumericVector &arrayY, NumericVector &aLengths, Rect &extent, RasterImage* pIntersectImage)
{
   bool bok = true;
   Rect rectbmp;
   map<TileIndex, RasterBitmap*>::iterator iBitmap;

   GdiFlush();

   // Get extent of the polygon being drawn

   if (extent.isNull())
   {
      extent = GetPolygonExtent(arrayX, arrayY, aLengths);
   };

   // Store extent of all polygons

   UpdateTotalExtent(extent);

   // Iterate through tiles of the bitmap

   int nTileSize = TILESIZE * m_nResolution;

   for (int n = extent.left/nTileSize; n <= extent.right/nTileSize && bok; n++)
   {
      for (int m = extent.top/nTileSize; m <= extent.bottom/nTileSize && bok; m++)
      {
         rectbmp.left = TILESIZE*n;
         rectbmp.top = TILESIZE*m;
         rectbmp.bottom = rectbmp.top - TILESIZE; //!!Check
         rectbmp.right = rectbmp.left + TILESIZE;

         // Check if the intersect image (if supplied) includes the tile

         if (pIntersectImage == NULL || pIntersectImage->m_aRaster.find(TileIndex(n,m)) != m_aRaster.end())
         {
            // Check if interesect polygon contains tile, otherwise don't need to draw

            if (m_aRaster.find(TileIndex(n,m)) == m_aRaster.end())
            {
               RasterBitmap *pRasterBitmap = new RasterBitmap;

               if (pRasterBitmap != NULL)
               {
                  m_aRaster[TileIndex(n,m)] = pRasterBitmap;
                  pRasterBitmap->SetRasterImage(this);
                  bok = pRasterBitmap->Initialise(rectbmp.left, rectbmp.top);

               } else
               {
                  bok = false;
               }
            }
            if (bok) bok = m_aRaster[TileIndex(n,m)]->DrawPolygon(arrayX, arrayY, aLengths, rectbmp.left, rectbmp.top);
         }
      };
   };

   return bok;
}



///////////////////////////////////////////////////////////////////////////////
//
// Intersects pRasterImage with this so that only overlapping bits (AND operation)
// are retained
//

bool RasterImage::Intersect(RasterImage *pRasterImage, Rect rectIntersect)
{
   bool bok = true;

   GdiFlush();

   Rect rect = m_rectExtent;
   if (!rectIntersect.isNull())
   {
      rect = m_rectExtent.intersected(rectIntersect);

      // Ensure extent covers full bytes as these are later used in GetExtent
      rect.left = m_rectExtent.left;
      rect.right = m_rectExtent.right;
   }

   map<TileIndex, RasterBitmap*>::iterator iBitmap;

   for (iBitmap = m_aRaster.begin(); iBitmap != m_aRaster.end() && bok; iBitmap++)
   {
      if (pRasterImage->m_aRaster.find(iBitmap->first) != m_aRaster.end())
      {
         bok = m_aRaster[iBitmap->first]->Intersect(pRasterImage->m_aRaster[iBitmap->first], rect);
      } else
      {
         iBitmap->second->Clear();
      }
   };
   return bok;
}

///////////////////////////////////////////////////////////////////////////////

void RasterImage::UpdateTotalExtent(Rect extent)
{
   m_rectExtent = m_rectExtent.united(extent);
}

///////////////////////////////////////////////////////////////////////////////

Rect RasterImage::GetPolygonExtent(NumericVector &arrayX, NumericVector &arrayY, NumericVector &aLengths)
{
   Rect extent;
   int istart = 0;

   for (int j = 0; j < aLengths.length(); j++)
   {
      for (int i = istart; i < aLengths[j]; i++)
      {
         if (j == 0 && i == 0) {extent = Rect(arrayX[i], arrayY[i], 0, 0);}

         if (arrayX[i] < extent.left) extent.left = arrayX[i];
         if (arrayX[i] > extent.right) extent.right = arrayX[i];
         if (arrayX[i] < extent.top) extent.top = arrayY[i];
         if (arrayX[i] > extent.bottom) extent.bottom = arrayY[i];
      };
      istart = aLengths[j];
   };

   return extent;
}

///////////////////////////////////////////////////////////////////////////////

RasterBitmap::RasterBitmap()
{
   m_nWidth = 0;
   m_nHeight = 0;
   m_nWidthBytes = 0;
   m_nLeft = 0;
   m_nTop = 0;
   m_pData = NULL;

   m_pRasterImage = NULL;

   m_hbitmap = NULL;
   m_hbitmapold = NULL;
   m_hdc = NULL;
}

////////////////////////////////////////////////////////////////////////////////

RasterBitmap::~RasterBitmap()
{
   Clear();
}

////////////////////////////////////////////////////////////////////////////////

int RasterBitmap::GetResolution()
{
   return m_pRasterImage->m_nResolution;
}

////////////////////////////////////////////////////////////////////////////////
//
// Returns extent in hectares

double RasterBitmap::GetExtent(Rect extent, int jMin, int jMax)
{
   if (IsEmpty()) return 0;

   // Need to synchronise any GDI requests to ensure that area value is correct

   GdiFlush();

   // Calculate the boundary of the polygons covering the bitmap

   int iMin = 0;
   int iMax = m_nWidthBytes/sizeof(DWORD);
   if (extent.left/GetResolution() > m_nLeft) iMin = (extent.left/GetResolution() - m_nLeft)/BITSBYTE/(int)sizeof(DWORD);
   if (extent.right/GetResolution() < m_nLeft + m_nWidth) iMax = (extent.right/GetResolution() - m_nLeft)/BITSBYTE/(int)sizeof(DWORD)+1;

   // Count number of bits that are set

   double iCount = 0;
   for (int j = jMin; j < jMax; j++)
   {
      for (int i = iMin; i < iMax; i++)
      {
         DWORD word = *((unsigned long*)m_pData + i + j * m_nWidthBytes/sizeof(DWORD));

         if (word == 0xFFFF) // Optimisation, handle extreme cases
         {
            iCount += 16;
         }
         else
         {
            while (word != 0)
            {
               if (word & 1) iCount++;
               word = word >> 1;
            };
         };
      }
   };


   return ((double)iCount*GetResolution()*GetResolution())/10000.0;
}

///////////////////////////////////////////////////////////////////////////////

Rect RasterBitmap::GetRect()
{
   return Rect(m_nLeft, m_nTop, m_nWidth, m_nHeight);
}

//////////////////////////////////////////////////////////////////////////////////

bool RasterBitmap::Clear()
{
   if (m_hbitmapold != NULL) SelectObject(m_hdc, m_hbitmapold);
   if (m_hbitmap != NULL) DeleteObject(m_hbitmap);
   if (m_hbrushold != NULL) SelectObject(m_hdc, m_hbrushold);
   if (m_hpenold != NULL) SelectObject(m_hdc, m_hpenold);
   if (m_hdc != NULL) DeleteDC(m_hdc);

   m_hbitmap = NULL;
   m_pData = NULL;
   m_hdc = NULL;
   m_hbrushold = NULL;
   m_hpenold = NULL;

   return true;
}

//////////////////////////////////////////////////////////////////////////////////

bool RasterBitmap::Initialise(int nLeft, int nTop)
{
   BITMAPINFO bmi;

   // Check if already initialised

   if (m_hbitmap != NULL) return true;

   HBRUSH hbrush = m_pRasterImage->GetBrush();
   HPEN hpen = m_pRasterImage->GetPen();

   // Create device context and select brush and pen

   m_hdc = CreateCompatibleDC(NULL);
   m_hbrushold = (HBRUSH)SelectObject(m_hdc, hbrush);
   m_hpenold = (HPEN)SelectObject(m_hdc, hpen);

   if (m_hdc == NULL || m_hbrushold == NULL || m_hpenold == NULL) return false;

   // Free up previous resources

   memset(&bmi, 0, sizeof(bmi));
   bmi.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth         = TILESIZE;
   bmi.bmiHeader.biHeight        = TILESIZE;
   bmi.bmiHeader.biPlanes        = 1;
   bmi.bmiHeader.biBitCount      = 1;
   bmi.bmiHeader.biCompression   = BI_RGB;
   m_hbitmap = CreateDIBSection(m_hdc, &bmi, DIB_RGB_COLORS, &m_pData, NULL, 0);

   if (m_hbitmap != NULL)
   {
      m_hbitmapold = (HBITMAP)SelectObject(m_hdc, m_hbitmap);

      BITMAP bitmap;
      GetObject(m_hbitmap, sizeof(BITMAP), &bitmap);
      m_nWidthBytes = bitmap.bmWidthBytes;
      m_nWidth = bitmap.bmWidth;
      m_nHeight = bitmap.bmHeight;

      m_nTop = nTop;
      m_nLeft = nLeft;
   }

   if (m_hbitmap != NULL && m_hbitmapold != NULL) return true;
   else
   {
      return false;
   }
}

///////////////////////////////////////////////////////////////////////////////

double RasterBitmap::GetExtent(Rect extent)
{
   // Only count bits within extent of drawn polygons

   int jMin = 0;
   int jMax = m_nHeight;

   if (extent.top/GetResolution() > m_nTop) jMax = m_nHeight-(extent.top/GetResolution()-m_nTop);
   if (extent.bottom/GetResolution() < m_nTop + m_nHeight) jMin = m_nHeight - (extent.bottom/GetResolution() - m_nTop);

   return RasterBitmap::GetExtent(extent, jMin, jMax);
}

///////////////////////////////////////////////////////////////////////////////

bool RasterBitmap::DrawPolygon(NumericVector &arrayX, NumericVector &arrayY, NumericVector &aLengths, int left, int top)
{
   bool bOK = true;
   POINT point;
   vector<int> aCounts;
   vector<POINT> aPoints;
   aCounts.reserve(aLengths.size());

   if (m_hbitmap == NULL) return false;

   // Count the total length
   aPoints.reserve(arrayX.size());

   int nStart = 0;
   for (int j = 0; j < aLengths.size(); j++)
   {
      for (int i = nStart; i < aLengths[j]; i++)
      {
         point.x = (int)(arrayX[i]/GetResolution() - left);
         point.y = (int)(arrayY[i]/GetResolution() - top);

         aPoints.push_back(point);
      }
      aCounts.push_back(aLengths[j] - nStart);
      nStart = aLengths[j];
   };

   // Draw polygons

   if (aLengths.size() == 1)
   {
      if (Polygon(m_hdc, aPoints.data(), aPoints.size()) == FALSE)
      {
         bOK = false;
      }
   }
   else if (aLengths.size() > 1)
   {
      if (PolyPolygon(m_hdc, aPoints.data(), aCounts.data(), aLengths.size()) == FALSE)
      {
         bOK = false;

      }
   }

   return bOK;
}

///////////////////////////////////////////////////////////////////////////////

bool RasterBitmap::Intersect(RasterBitmap *pRasterBitmap, Rect extent)
{
   if (m_hbitmap == NULL)
   {
      return true;
   }
   else if (IsEmpty())
   {
      Clear();
      return true;
   }
   else
   {
      int x = 0;
      int y = 0;
      int cx = m_nWidth;
      int cy = m_nHeight;

      // Only bitblt for extent of drawn polygons

      if (extent.left/GetResolution() > m_nLeft) x = extent.left/GetResolution() - m_nLeft;
      if (extent.right/GetResolution() < m_nLeft + m_nWidth) cx = extent.right/GetResolution() - m_nLeft;
      cx -= x;

      if (extent.top/GetResolution() > m_nTop) y = extent.top/GetResolution()-m_nTop;
      if (extent.bottom/GetResolution() < m_nTop + m_nHeight) cy = extent.bottom/GetResolution() - m_nTop;
      cy -= y;


      return BitBlt(m_hdc,x,y,cx, cy, ((RasterBitmap*)pRasterBitmap)->m_hdc, x,y, SRCAND);
   };
}

