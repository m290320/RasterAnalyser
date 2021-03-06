// This file was generated by Rcpp::compileAttributes
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <Rcpp.h>

using namespace Rcpp;

// Initialise
XPtr<int> Initialise(int nResolution);
RcppExport SEXP RasterAnalyser_Initialise(SEXP nResolutionSEXP) {
BEGIN_RCPP
    Rcpp::RObject __result;
    Rcpp::RNGScope __rngScope;
    Rcpp::traits::input_parameter< int >::type nResolution(nResolutionSEXP);
    __result = Rcpp::wrap(Initialise(nResolution));
    return __result;
END_RCPP
}
// Destroy
XPtr<int> Destroy(XPtr<int> pRasterImage);
RcppExport SEXP RasterAnalyser_Destroy(SEXP pRasterImageSEXP) {
BEGIN_RCPP
    Rcpp::RObject __result;
    Rcpp::RNGScope __rngScope;
    Rcpp::traits::input_parameter< XPtr<int> >::type pRasterImage(pRasterImageSEXP);
    __result = Rcpp::wrap(Destroy(pRasterImage));
    return __result;
END_RCPP
}
// DrawPolygon
bool DrawPolygon(XPtr<int> pRasterImage, NumericVector arrayX, NumericVector arrayY, NumericVector aLengths);
RcppExport SEXP RasterAnalyser_DrawPolygon(SEXP pRasterImageSEXP, SEXP arrayXSEXP, SEXP arrayYSEXP, SEXP aLengthsSEXP) {
BEGIN_RCPP
    Rcpp::RObject __result;
    Rcpp::RNGScope __rngScope;
    Rcpp::traits::input_parameter< XPtr<int> >::type pRasterImage(pRasterImageSEXP);
    Rcpp::traits::input_parameter< NumericVector >::type arrayX(arrayXSEXP);
    Rcpp::traits::input_parameter< NumericVector >::type arrayY(arrayYSEXP);
    Rcpp::traits::input_parameter< NumericVector >::type aLengths(aLengthsSEXP);
    __result = Rcpp::wrap(DrawPolygon(pRasterImage, arrayX, arrayY, aLengths));
    return __result;
END_RCPP
}
// Intersect
bool Intersect(XPtr<int> pRasterImage1, XPtr<int> pRasterImage2, NumericVector rectIntersect);
RcppExport SEXP RasterAnalyser_Intersect(SEXP pRasterImage1SEXP, SEXP pRasterImage2SEXP, SEXP rectIntersectSEXP) {
BEGIN_RCPP
    Rcpp::RObject __result;
    Rcpp::RNGScope __rngScope;
    Rcpp::traits::input_parameter< XPtr<int> >::type pRasterImage1(pRasterImage1SEXP);
    Rcpp::traits::input_parameter< XPtr<int> >::type pRasterImage2(pRasterImage2SEXP);
    Rcpp::traits::input_parameter< NumericVector >::type rectIntersect(rectIntersectSEXP);
    __result = Rcpp::wrap(Intersect(pRasterImage1, pRasterImage2, rectIntersect));
    return __result;
END_RCPP
}
// GetExtent
double GetExtent(XPtr<int> pRasterImage, NumericVector rectIntersect);
RcppExport SEXP RasterAnalyser_GetExtent(SEXP pRasterImageSEXP, SEXP rectIntersectSEXP) {
BEGIN_RCPP
    Rcpp::RObject __result;
    Rcpp::RNGScope __rngScope;
    Rcpp::traits::input_parameter< XPtr<int> >::type pRasterImage(pRasterImageSEXP);
    Rcpp::traits::input_parameter< NumericVector >::type rectIntersect(rectIntersectSEXP);
    __result = Rcpp::wrap(GetExtent(pRasterImage, rectIntersect));
    return __result;
END_RCPP
}
// Clear
void Clear(XPtr<int> pRasterImage);
RcppExport SEXP RasterAnalyser_Clear(SEXP pRasterImageSEXP) {
BEGIN_RCPP
    Rcpp::RNGScope __rngScope;
    Rcpp::traits::input_parameter< XPtr<int> >::type pRasterImage(pRasterImageSEXP);
    Clear(pRasterImage);
    return R_NilValue;
END_RCPP
}
