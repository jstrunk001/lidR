/*
 ===============================================================================

 PROGRAMMERS:

jean-romain.roussel.1@ulaval.ca  -  https://github.com/Jean-Romain/lidR

COPYRIGHT:

Copyright 2018 Jean-Romain Roussel

This file is part of lidR R package.

lidR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>

===============================================================================
*/

// [[Rcpp::depends(BH)]]
#include <Rcpp.h>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include "myomp.h"

using namespace Rcpp;

typedef boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian> Point;
typedef boost::geometry::model::polygon<Point> Polygon;
typedef boost::geometry::model::multi_polygon<Polygon> MultiPolygon;

// [[Rcpp::export]]
LogicalVector C_points_in_polygon_wkt(NumericVector x, NumericVector y, std::string wkt, int ncpu)
{
  if (x.length() != y.length())
    throw std::runtime_error("Unexpected error in point in polygon: x and y are not the same length.");

  int npoints = x.length();
  LogicalVector in_poly(npoints);

  if (wkt.find("MULTIPOLYGON") != std::string::npos)
  {
    MultiPolygon polygons;
    boost::geometry::read_wkt(wkt, polygons);

    #pragma omp parallel for num_threads(ncpu)
    for(int i = 0 ; i < npoints ; i++)
    {
      Point p;
      p.set<0>(x[i]);
      p.set<1>(y[i]);

      bool isin = boost::geometry::covered_by(p, polygons);

      #pragma omp critical
      {
        in_poly[i] = isin;
      }
    }
  }
  else if (wkt.find("POLYGON") != std::string::npos)
  {
    Polygon polygon;
    boost::geometry::read_wkt(wkt, polygon);

    #pragma omp parallel for num_threads(ncpu)
    for(int i = 0 ; i < npoints ; i++)
    {
      Point p;
      p.set<0>(x[i]);
      p.set<1>(y[i]);

      bool isin = boost::geometry::covered_by(p, polygon);

      #pragma omp critical
      {
        in_poly[i] = isin;
      }
    }
  }
  else
    throw std::runtime_error("Unexpected error in point in polygon: WKT is not a POLYGON or MULTIPOLYGON");

  return in_poly;
}
