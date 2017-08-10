#include <stdio.h>
#include <string>
#include <vector>
#include <dirent.h>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include "estimator.hpp"

//RANSAC variations
#include "RANSAC.cpp"
#include "PROSAC.cpp"
#include "TddTest.cpp"
#include "PE-RANSAC.cpp"
#include "LO-RANSAC.cpp"

//COnstants
#include "../constants.hpp"

using namespace std;
using namespace cv;

int Estimator::num_iters = 0;
float Estimator::runtime = 0;
vector<int> Estimator::pool = vector<int>();

void Estimator::subselect(_InputArray _points1, _InputArray _points2, _OutputArray _output1, _OutputArray _output2, int len, int limit){
	Mat points1 = _points1.getMat(), points2 = _points2.getMat();
	int d1 = points1.channels() > 1 ? points1.channels() : points1.cols;
	bool multichan = points1.channels() > 1;
	int count = points1.checkVector(d1), count2 = points1.checkVector(d1);

	CV_Assert( points1.type() == points2.type() );
	CV_Assert( count == count2 );

	if(limit <= 0)
		limit = count;

	if(multichan){
		_output1.create(len, 1, points1.type());
		_output2.create(len, 1, points2.type());
	} else {
		_output1.create(len, 2, points1.type());
		_output2.create(len, 2, points2.type());
	}

	Mat output1 = _output1.getMat();
	Mat output2 = _output2.getMat();

	if(Estimator::pool.size() != count){
		Estimator::pool = vector<int>(count);
		for (int i=0; i<count; i++) Estimator::pool[i] = i;
	}

	random_shuffle(Estimator::pool.begin(), Estimator::pool.begin()+limit);
	
	for(int j=0; j<len; j++){
		points1.row(Estimator::pool[j]).copyTo(output1.row(j));
		points2.row(Estimator::pool[j]).copyTo(output2.row(j));
	}

}

//Directly ported from opencv ptsetreg.cpp
int Estimator::updateNumIters( double p, double ep, int modelPoints, int maxIters )
{

    p = MAX(p, 0.);
    p = MIN(p, 1.);
    ep = MAX(ep, 0.);
    ep = MIN(ep, 1.);

    // avoid inf's & nan's
    double num = max(1. - p, DBL_MIN);
    double denom = 1. - pow(1. - ep, modelPoints);
    if( denom < DBL_MIN )
        return 0;

    num = log(num);
    denom = log(denom);

    return denom >= 0 || -num >= maxIters*(-denom) ? maxIters : cvRound(num/denom);
}

//Directly ported from opencv
int Estimator::getInliers(_InputArray _points1, _InputArray _points2, Mat _F, double err, _OutputArray _mask){

	Mat __m1 = _points1.getMat(), __m2 = _points2.getMat();
    int i, count = __m1.checkVector(2);

    const Point2f* m1 = __m1.ptr<Point2f>();
    const Point2f* m2 = __m2.ptr<Point2f>();
    const double* F = _F.ptr<double>();

    uchar* mask;
    if(_mask.needed()){
    	_mask.create(count, 1, CV_8U);
    	mask = _mask.getMat().ptr<uchar>();
	} else {
		Mat tempMask;
		tempMask.create(count, 1, CV_8U);
		mask = tempMask.ptr<uchar>();
	}

    float thresh = (float)(err*err);
    int nInliers = 0;


    for( i = 0; i < count; i++ )
    {
        double a, b, c, d1, d2, s1, s2;

        a = F[0]*m1[i].x + F[1]*m1[i].y + F[2];
        b = F[3]*m1[i].x + F[4]*m1[i].y + F[5];
        c = F[6]*m1[i].x + F[7]*m1[i].y + F[8];

        s2 = 1./(a*a + b*b);
        d2 = m2[i].x*a + m2[i].y*b + c;

        a = F[0]*m2[i].x + F[3]*m2[i].y + F[6];
        b = F[1]*m2[i].x + F[4]*m2[i].y + F[7];
        c = F[2]*m2[i].x + F[5]*m2[i].y + F[8];

        s1 = 1./(a*a + b*b);
        d1 = m1[i].x*a + m1[i].y*b + c;

        if(mask[i] = ((float)max(d1*d1*s1, d2*d2*s2) <= thresh))
        	nInliers++;
    }

    return nInliers;
}

//Directly ported from SFM opencv module
bool Estimator::essenMat(_InputArray _F, _OutputArray _E){
	return true;
}

//Directly ported from opencv
int Estimator::fundMat(_InputArray _points1, _InputArray _points2, _OutputArray _F, bool useAll = false){

	Mat _m1 = _points1.getMat(), _m2 = _points2.getMat();
	Point2d m1c(0,0), m2c(0,0);
    double t, scale1 = 0, scale2 = 0;

    const Point2f* m1 = _m1.ptr<Point2f>();
    const Point2f* m2 = _m2.ptr<Point2f>();
    CV_Assert( (_m1.cols == 1 || _m1.rows == 1) && _m1.size() == _m2.size());
    int i, count = _m1.checkVector(2);

    if(!useAll)
    	count = (count < MIN_MODEL_POINTS)? count : MIN_MODEL_POINTS;

    // compute centers and average distances for each of the two point sets
    for( i = 0; i < count; i++ )
    {
        m1c += Point2d(m1[i].x, m1[i].y);
        m2c += Point2d(m2[i].x, m2[i].y);
    }

    t = 1./count;
    m1c *= t;
    m2c *= t;

    for( i = 0; i < count; i++ )
    {
        scale1 += norm(Point2d(m1[i].x - m1c.x, m1[i].y - m1c.y));
        scale2 += norm(Point2d(m2[i].x - m2c.x, m2[i].y - m2c.y));
    }

    scale1 *= t;
    scale2 *= t;

    if( scale1 < FLT_EPSILON || scale2 < FLT_EPSILON )
        return 0;

    scale1 = sqrt(2.)/scale1;
    scale2 = sqrt(2.)/scale2;

    Matx<double, 9, 9> A;

    for( i = 0; i < count; i++ )
    {
        double x1 = (m1[i].x - m1c.x)*scale1;
        double y1 = (m1[i].y - m1c.y)*scale1;
        double x2 = (m2[i].x - m2c.x)*scale2;
        double y2 = (m2[i].y - m2c.y)*scale2;
        Vec<double, 9> r( x2*x1, x2*y1, x2, y2*x1, y2*y1, y2, x1, y1, 1 );
        A += r*r.t();
    }

    Vec<double, 9> W;
    Matx<double, 9, 9> V;

    eigen(A, W, V);

    for( i = 0; i < 9; i++ )
    {
        if( fabs(W[i]) < DBL_EPSILON )
            break;
    }

    if( i < 8 )
        return 0;

    Matx33d F0( V.val + 9*8 );

    Vec3d w;
    Matx33d U;
    Matx33d Vt;

    SVD::compute( F0, w, U, Vt);
    w[2] = 0.;

    F0 = U*Matx33d::diag(w)*Vt;

    Matx33d T1( scale1, 0, -scale1*m1c.x, 0, scale1, -scale1*m1c.y, 0, 0, 1 );
    Matx33d T2( scale2, 0, -scale2*m2c.x, 0, scale2, -scale2*m2c.y, 0, 0, 1 );

    F0 = T2.t()*F0*T1;

    // make F(3,3) = 1
    if( fabs(F0(2,2)) > FLT_EPSILON )
        F0 *= 1./F0(2,2);


    Mat(F0).copyTo(_F);

    return 1;
}

void Estimator::debug(_InputArray _points1, _InputArray _points2, _OutputArray _F){
	Estimator::fundMat(_points1.getMat(), _points2.getMat(), _F);
}

Mat Estimator::estFundamentalMat(_InputArray _points1, _InputArray _points2,
                                int method, double param1 = 1, double param2 = 0.99, _OutputArray _mask = _OutputArray()){
	return Estimator::estFundamentalMat(_points1, _points2, method, param1, param2, INLIER_RATIO, _mask, vector<float>());
}
Mat Estimator::estFundamentalMat(_InputArray _points1, _InputArray _points2,
                                int method, double param1 = 1, double param2 = 0.99, _OutputArray _mask = _OutputArray(), vector<float> similarities = vector<float>()){
	return Estimator::estFundamentalMat(_points1, _points2, method, param1, param2, INLIER_RATIO, _mask, similarities);
}
Mat Estimator::estFundamentalMat(_InputArray _points1, _InputArray _points2,
                                int method, double param1 = 1, double param2 = 0.99, double param3 = INLIER_RATIO, _OutputArray _mask = _OutputArray()){
	return Estimator::estFundamentalMat(_points1, _points2, method, param1, param2, param3, _mask, vector<float>());
}

Mat Estimator::estFundamentalMat(_InputArray _points1, _InputArray _points2,
                                int method, double param1 = 1, double param2 = 0.99, double param3 = INLIER_RATIO, _OutputArray _mask = _OutputArray(), vector<float> similarities = vector<float>()){
		
	//Get Matrices
	Mat points1 = _points1.getMat(), points2 = _points2.getMat();
    Mat m1, m2, F, nm1, nm2, T1, T2;

    //Number of points, determines if algorithm could be run
    int npoints = -1;

    for( int i = 1; i <= 2; i++ )
    {
        Mat& p = i == 1 ? points1 : points2;
        Mat& m = i == 1 ? m1 : m2;
        npoints = p.checkVector(2, -1, false);
        if( npoints < 0 )
        {
            npoints = p.checkVector(3, -1, false);
            //if( npoints < 0 )
                //CV_Error(Error::StsBadArg, "The input arrays should be 2D point sets");
            if( npoints == 0 )
                return Mat();
            convertPointsFromHomogeneous(p, p);
        }
        p.reshape(2, npoints).convertTo(m, CV_32F);
    }

    CV_Assert( m1.checkVector(2) == m2.checkVector(2) );

    //cout << npoints << endl;
    _mask.create(npoints, 1, CV_8U, -1, true);
    Mat mask = _mask.getMat();
    CV_Assert( (mask.cols == 1 || mask.rows == 1) && (int)mask.total() == npoints );
    mask.setTo(Scalar::all(1));

    // Result, returns number of inliers
	int result = 0;

    FundMatEstimator* fme = createFundMatEstimator(method, param1, param2, param3);
    fme->run(m1, m2, F, _mask, similarities);
    Estimator::num_iters = fme->lastNumOfIterations();
    Estimator::runtime = fme->lastRuntime();
    //cout << result << endl;

    //IN case if inliers are lower than required
    if( result < 8)
    	return Mat();

	return F;
}

FundMatEstimator* Estimator::createFundMatEstimator(int method, double param1, double param2, double param3 = INLIER_RATIO){
	switch(method) {
		case SFM_RANSAC :
			return (new RANSAC_Estimator(param1, param2, param3));
			break;
		case SFM_RANSAC_Tdd :
			return (new TddTest_Estimator(param1, param2, param3, false));
			break;
		case SFM_PROSAC_Tdd :
			return (new TddTest_Estimator(param1, param2, param3, true));
			break;
		case SFM_PE_RANSAC :
			return (new PE_RANSAC_Estimator(param1, param2, param3));
			break;
		case SFM_PROSAC :
			return (new PROSAC_Estimator(param1, param2, param3));
			break;
		case SFM_LO_RANSAC :
			return (new LO_RANSAC_Estimator(param1, param2, param3));
			break;
		default :
			return NULL;
	}
}