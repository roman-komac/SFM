#include <stdio.h>
#include <string>
#include <vector>
#include <dirent.h>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include "ransac.hpp"
#include "../constants.hpp"

//By default we set the inlier ratio to 40% (60% outlier contamination)
#define INLIER_RATIO 0.4

//Minimum required points for linear equation solving
#define MIN_MODEL_POINTS 8

using namespace std;
using namespace cv;

int Estimator::num_iters = 0;
float Estimator::runtime = 0;

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
int Estimator::getInliers(_InputArray _points1, _InputArray _points2, Mat _F, double err, _OutputArray _mask = noArray()){

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
    num_iters = fme->lastNumOfIterations();
    runtime = fme->lastRuntime();
    //cout << result << endl;

    //IN case if inliers are lower than required
    if( result < 8)
    	return Mat();

	return F;
}


class RANSAC_Estimator : public FundMatEstimator{
	public:
		RANSAC_Estimator(double param1, double param2, double param3){
			reprojectError = param1;
			confidence = param2;
			inlier_ratio = param3;
		}
		int run(_InputArray _points1, _InputArray _points2, _OutputArray _F, _OutputArray _mask = noArray(), vector<float> similarities = vector<float>()){

			//Reset number of iterations and runtime
			num_iters = 0;
			loop_runtime = 0;

			Mat points1 = _points1.getMat(), points2 = _points2.getMat();
			Mat mask, model, bestModel, subselect1, subselect2, bestMask1, bestMask2;

	        //Number of iterations set according to the standard termination criterion
	        int iter, niters = (int)ceil(log(1 - confidence)/log(1 - pow(inlier_ratio,8)));
	        int d1 = points1.channels() > 1 ? points1.channels() : points1.cols;
	        int d2 = points2.channels() > 1 ? points2.channels() : points2.cols;
	        int count = points1.checkVector(d1), count2 = points2.checkVector(d2), maxGoodCount = 0;

	        CV_Assert( confidence > 0 && confidence < 1 );
	        CV_Assert( count >= 0 && count2 == count );
	        if( count < MIN_MODEL_POINTS )
	            return 0;

	        if( _mask.needed() )
	        {
	            _mask.create(count, 1, CV_8U, -1, true);
	            bestMask2 = bestMask1 = _mask.getMat();
	            CV_Assert( (bestMask1.cols == 1 || bestMask1.rows == 1) && (int)bestMask1.total() == count );
	        }
	        else
	        {
	            bestMask1.create(count, 1, CV_8U);
	            bestMask2 = bestMask1;
	        }

	        if( count == MIN_MODEL_POINTS )
	        {
	            Estimator::fundMat(points1, points2, bestModel, true);
	            bestModel.copyTo(_F);
	            bestMask1.setTo(Scalar::all(1));
	            return MIN_MODEL_POINTS;
	        }

	        clock_t t1,t2;
		    t1=clock();
	        for( iter = 0; iter < niters; iter++ )
	        {
	        	//For benchmarking
	        	num_iters++;

	        	subselect(points1, points2, subselect1, subselect2);
	            int i, nmodels;

                nmodels = Estimator::fundMat(subselect1, subselect2, model, true);
	            if( nmodels <= 0 )
	                continue;
	            CV_Assert( model.rows % nmodels == 0 );
	            Size modelSize(model.cols, model.rows/nmodels);

	            for( i = 0; i < nmodels; i++ )
	            {
	                Mat model_i = model.rowRange( i*modelSize.height, (i+1)*modelSize.height );
	                int goodCount = Estimator::getInliers( points1, points2, model, reprojectError, mask );

	                if( goodCount > max(maxGoodCount, MIN_MODEL_POINTS-1) )
	                {
	                    swap(mask, bestMask1);
	                    model_i.copyTo(bestModel);
	                    maxGoodCount = goodCount;
	                    niters = Estimator::updateNumIters( confidence, (double)(count - goodCount)/count, MIN_MODEL_POINTS, niters );
	                }
				}
	            
	        }
	        t2=clock();
		    loop_runtime = ((float)t2-(float)t1)/CLOCKS_PER_SEC;

	        if( maxGoodCount > 0 )
	        {
	            if( bestMask1.data != bestMask2.data )
	            {
	                if( bestMask1.size() == bestMask2.size() )
	                    bestMask1.copyTo(bestMask2);
	                else
	                    transpose(bestMask1, bestMask2);
	            }
	            bestModel.copyTo(_F);
	        }
	        else
	            _F.release();

			return maxGoodCount;

		}
	private:
		void subselect(_InputArray _points1, _InputArray _points2, _OutputArray _output1, _OutputArray _output2, vector<float> similarities = vector<float>()){
			
			Mat points1 = _points1.getMat(), points2 = _points2.getMat();
			int d1 = points1.channels() > 1 ? points1.channels() : points1.cols;
			bool multichan = points1.channels() > 1;
			int count = points1.checkVector(d1);

			CV_Assert( points1.type() == points2.type() );

			if(multichan){
				_output1.create(MIN_MODEL_POINTS, 1, points1.type());
				_output2.create(MIN_MODEL_POINTS, 1, points2.type());
			} else {
				_output1.create(MIN_MODEL_POINTS, 2, points1.type());
				_output2.create(MIN_MODEL_POINTS, 2, points2.type());
			}

			Mat output1 = _output1.getMat();
			Mat output2 = _output2.getMat();

			if(selection.size() != count){
				selection = vector<int>();
				for (int i=0; i<count; i++) selection.push_back(i);
			}

			random_shuffle(selection.begin(), selection.end());
			for(int j=0; j<MIN_MODEL_POINTS; j++){
				points1.row(selection[j]).copyTo(output1.row(j));
				points2.row(selection[j]).copyTo(output2.row(j));
			}
		}
};
class PE_RANSAC_Estimator : public FundMatEstimator{
	public:
		PE_RANSAC_Estimator(double param1, double param2, double param3){
			reprojectError = param1;
			confidence = param2;
			inlier_ratio = param3;
			//Recommended values
			//M = number of generated hypotheses
			//B = number of points evaluated each time
			M = 500;
			B = 100;
		}
		int run(_InputArray _points1, _InputArray _points2, _OutputArray F, _OutputArray _mask = _OutputArray(), vector<float> similarities = vector<float>()){

			//Reset number of iterations and runtime
			num_iters = 0;
			loop_runtime = 0;
			return 0;
		}
	private:
		void subselect(_InputArray _points1, _InputArray _points2, _OutputArray _output1, _OutputArray _output2, vector<float> similarities = vector<float>()){

		}
};
class PROSAC_Estimator : public FundMatEstimator{
	public:
		PROSAC_Estimator(double param1, double param2, double param3){
			reprojectError = param1;
			confidence = param2;
			inlier_ratio = param3;

		}
		int run(_InputArray _points1, _InputArray _points2, _OutputArray _F, _OutputArray _mask = _OutputArray(), vector<float> similarities = vector<float>()){
			if(!similarities.size()){
					cout << "similarity scores required to run SFM_PROSAC estimator" << endl;
				CV_Assert( similarities.size() );
			}
			Mat points1 = _points1.getMat(), points2 = _points2.getMat();
			Mat mask, model, bestModel, subselect1, subselect2, bestMask1, bestMask2;

			//Reset number of iterations and runtime
			num_iters = 0;
			loop_runtime = 0;

	        //Number of iterations set according to the standard termination criterion
	        int iter, niters = (int)ceil(log(1 - confidence)/log(1 - pow(inlier_ratio,8)));
	        int d1 = points1.channels() > 1 ? points1.channels() : points1.cols;
	        int d2 = points2.channels() > 1 ? points2.channels() : points2.cols;
	        int count = points1.checkVector(d1), count2 = points2.checkVector(d2), maxGoodCount = 0;

	        CV_Assert( confidence > 0 && confidence < 1 );
	        CV_Assert( count >= 0 && count2 == count );
	        if( count < MIN_MODEL_POINTS )
	            return 0;

	        if( _mask.needed() )
	        {
	            _mask.create(count, 1, CV_8U, -1, true);
	            bestMask2 = bestMask1 = _mask.getMat();
	            CV_Assert( (bestMask1.cols == 1 || bestMask1.rows == 1) && (int)bestMask1.total() == count );
	        }
	        else
	        {
	            bestMask1.create(count, 1, CV_8U);
	            bestMask2 = bestMask1;
	        }

	        if( count == MIN_MODEL_POINTS )
	        {
	            Estimator::fundMat(points1, points2, bestModel, true);
	            bestModel.copyTo(_F);
	            bestMask1.setTo(Scalar::all(1));
	            return MIN_MODEL_POINTS;
	        }

	        currIter = 0;
	        stageIters = 1;
	        stageN = MIN_MODEL_POINTS;
	        Tn = niters;
			for (int i = 0; i < MIN_MODEL_POINTS; i++) {
				Tn *= MIN_MODEL_POINTS-i;
				Tn /= count-i;
			}

			clock_t t1,t2;
		    t1=clock();
	        for( iter = 0; iter < niters; iter++ )
	        {
	        	//For benchmarking
	        	num_iters++;

	        	subselect(points1, points2, subselect1, subselect2);
	            int i, nmodels;

                nmodels = Estimator::fundMat(subselect1, subselect2, model, false); 
	            if( nmodels <= 0 )
	                continue;
	            CV_Assert( model.rows % nmodels == 0 );
	            Size modelSize(model.cols, model.rows/nmodels);

	            for( i = 0; i < nmodels; i++ )
	            {
	                Mat model_i = model.rowRange( i*modelSize.height, (i+1)*modelSize.height );

	                int goodCount = Estimator::getInliers( points1, points2, model, reprojectError, mask );

	                if( goodCount > max(maxGoodCount, MIN_MODEL_POINTS-1) )
	                {
	                    swap(mask, bestMask1);
	                    model_i.copyTo(bestModel);
	                    maxGoodCount = goodCount;
	                    niters = Estimator::updateNumIters( confidence, (double)(count - goodCount)/count, MIN_MODEL_POINTS, niters );
	                }
		            
				}
	        }
	        t2=clock();
		    loop_runtime = ((float)t2-(float)t1)/CLOCKS_PER_SEC;

	        if( maxGoodCount > 0 )
	        {
	            if( bestMask1.data != bestMask2.data )
	            {
	                if( bestMask1.size() == bestMask2.size() )
	                    bestMask1.copyTo(bestMask2);
	                else
	                    transpose(bestMask1, bestMask2);
	            }
	            bestModel.copyTo(_F);
	        }
	        else
	            _F.release();

			return maxGoodCount;

			return 0;
		}
	private:
		//Max number of samples drawn
		double Tn;
		//Current iterations
		int stageIters;
		int currIter;
		//Current stage number of sample points
		int stageN;

		void subselect(_InputArray _points1, _InputArray _points2, _OutputArray _output1, _OutputArray _output2, vector<float> similarities = vector<float>()){
			Mat points1 = _points1.getMat(), points2 = _points2.getMat();
			int d1 = points1.channels() > 1 ? points1.channels() : points1.cols;
			bool multichan = points1.channels() > 1;
			int count = points1.checkVector(d1);

			CV_Assert( points1.type() == points2.type() );

			if(multichan){
				_output1.create(MIN_MODEL_POINTS, 1, points1.type());
				_output2.create(MIN_MODEL_POINTS, 1, points2.type());
			} else {
				_output1.create(MIN_MODEL_POINTS, 2, points1.type());
				_output2.create(MIN_MODEL_POINTS, 2, points2.type());
			}

			Mat output1 = _output1.getMat();
			Mat output2 = _output2.getMat();

			if(selection.size() != count){
				selection = vector<int>();
				for (int i=0; i<count; i++) selection.push_back(i);
			}

			currIter++;

			if (currIter >= stageIters && stageN < count) {
				stageN++;

				double Tn1 = Tn * (double)(stageN+1) / (stageN+1-MIN_MODEL_POINTS);
				double stageIts = Tn1 - Tn;
				stageIters = (int)ceil(stageIts);
				currIter = 0;
				Tn = Tn1;
			}


			random_shuffle(selection.begin(), selection.begin()+stageN);
			for(int j=0; j < MIN_MODEL_POINTS; j++){
				points1.row(selection[j]).copyTo(output1.row(j));
				points2.row(selection[j]).copyTo(output2.row(j));
			}
		}
};
class MLESAC_Estimator : public FundMatEstimator{
	public:
		MLESAC_Estimator(double param1, double param2, double param3){
			reprojectError = param1;
			confidence = param2;
			inlier_ratio = param3;
		}
		int run(_InputArray _points1, _InputArray _points2, _OutputArray F, _OutputArray _mask = _OutputArray(), vector<float> similarities = vector<float>()){

			//Reset number of iterations and runtime
			num_iters = 0;
			loop_runtime = 0;
			return 0;
		}
	private:
		void subselect(_InputArray _points1, _InputArray _points2, _OutputArray _output1, _OutputArray _output2, vector<float> similarities = vector<float>()){

		}
};
class ARRSAC_Estimator : public FundMatEstimator{
	public:
		ARRSAC_Estimator(double param1, double param2, double param3){
			reprojectError = param1;
			confidence = param2;
			inlier_ratio = param3;
		}
		int run(_InputArray _points1, _InputArray _points2, _OutputArray F, _OutputArray _mask = _OutputArray(), vector<float> similarities = vector<float>()){

			//Reset number of iterations and runtime
			num_iters = 0;
			loop_runtime = 0;

			return 0;
		}
	private:
		void subselect(_InputArray _points1, _InputArray _points2, _OutputArray _output1, _OutputArray _output2, vector<float> similarities = vector<float>()){

		}
};

class TddTest_Estimator : public FundMatEstimator{
	public:
		//Whether to use a priori known point similarity data
		//If set to true, optional parameter similarities is required
		bool useSimilarityScore;
		TddTest_Estimator(double param1, double param2, double param3, bool simScore){
			reprojectError = param1;
			confidence = param2;
			inlier_ratio = param3;
			useSimilarityScore = simScore;
			//Reccomended value
			//D = number of extra points to take in the pre-verification
			D = 1;
		}
		int run(_InputArray _points1, _InputArray _points2, _OutputArray _F, _OutputArray _mask = _OutputArray(), vector<float> similarities = vector<float>()){
			
			//Reset number of iterations and runtime
			num_iters = 0;
			loop_runtime = 0;

			//Number of iterations set according to the standard termination criterion
	        int iter, niters = (int)ceil(log(1 - confidence)/log(1 - pow(inlier_ratio,8)));
			Mat points1 = _points1.getMat(), points2 = _points2.getMat();
			Mat mask, model, bestModel, subselect1, subselect2, bestMask1, bestMask2;

	        int d1 = points1.channels() > 1 ? points1.channels() : points1.cols;
	        int d2 = points2.channels() > 1 ? points2.channels() : points2.cols;
	        int count = points1.checkVector(d1), count2 = points2.checkVector(d2), maxGoodCount = 0;

	        CV_Assert( confidence > 0 && confidence < 1 );
	        CV_Assert( count >= 0 && count2 == count );
	        if( count < MIN_MODEL_POINTS )
	            return 0;


	        if(useSimilarityScore){
				if(!similarities.size())
					cout << "similarity scores required to run SFM_RANSAC_Tdd_SScore estimator" << endl;
				CV_Assert( similarities.size() );

				currIter = 0;
		        stageIters = 1;
		        stageN = MIN_MODEL_POINTS;
		        Tn = niters;
				for (int i = 0; i < MIN_MODEL_POINTS; i++) {
					Tn *= MIN_MODEL_POINTS-i;
					Tn /= count-i;
				}
			}

	        if( _mask.needed() )
	        {
	            _mask.create(count, 1, CV_8U, -1, true);
	            bestMask2 = bestMask1 = _mask.getMat();
	            CV_Assert( (bestMask1.cols == 1 || bestMask1.rows == 1) && (int)bestMask1.total() == count );
	        }
	        else
	        {
	            bestMask1.create(count, 1, CV_8U);
	            bestMask2 = bestMask1;
	        }

	        if( count == MIN_MODEL_POINTS )
	        {
	            Estimator::fundMat(points1, points2, bestModel, true);
	            bestModel.copyTo(_F);
	            bestMask1.setTo(Scalar::all(1));
	            return MIN_MODEL_POINTS;
	        }

	        clock_t t1,t2;
		    t1=clock();
	        for( iter = 0; iter < niters; iter++ )
	        {
	        	//For benchmarking
	        	num_iters++;

	        	subselect(points1, points2, subselect1, subselect2);
	            int i, nmodels;

                nmodels = Estimator::fundMat(subselect1, subselect2, model, false); 
	            if( nmodels <= 0 )
	                continue;
	            CV_Assert( model.rows % nmodels == 0 );
	            Size modelSize(model.cols, model.rows/nmodels);

	            for( i = 0; i < nmodels; i++ )
	            {
	                Mat model_i = model.rowRange( i*modelSize.height, (i+1)*modelSize.height );
	                if(Estimator::getInliers( subselect1, subselect2, model, reprojectError) == (MIN_MODEL_POINTS+D)){
		                int goodCount = Estimator::getInliers( points1, points2, model, reprojectError, mask );

		                if( goodCount > max(maxGoodCount, MIN_MODEL_POINTS-1) )
		                {
		                    swap(mask, bestMask1);
		                    model_i.copyTo(bestModel);
		                    maxGoodCount = goodCount;
		                    niters = Estimator::updateNumIters( confidence, (double)(count - goodCount)/count, MIN_MODEL_POINTS, niters );
		                }
		            }
				}
	        }
	        t2=clock();
		    loop_runtime = ((float)t2-(float)t1)/CLOCKS_PER_SEC;

	        if( maxGoodCount > 0 )
	        {
	            if( bestMask1.data != bestMask2.data )
	            {
	                if( bestMask1.size() == bestMask2.size() )
	                    bestMask1.copyTo(bestMask2);
	                else
	                    transpose(bestMask1, bestMask2);
	            }
	            bestModel.copyTo(_F);
	        }
	        else
	            _F.release();

			return maxGoodCount;

			return 0;
		}
	private:
		//Max number of samples drawn
		double Tn;
		//Current iterations
		int stageIters;
		int currIter;
		//Current stage number of sample points
		int stageN;
		void subselect(_InputArray _points1, _InputArray _points2, _OutputArray _output1, _OutputArray _output2, vector<float> similarities = vector<float>()){
			Mat points1 = _points1.getMat(), points2 = _points2.getMat();
			int d1 = points1.channels() > 1 ? points1.channels() : points1.cols;
			bool multichan = points1.channels() > 1;
			int count = points1.checkVector(d1);

			CV_Assert( points1.type() == points2.type() );

			if(multichan){
				_output1.create(MIN_MODEL_POINTS+D, 1, points1.type());
				_output2.create(MIN_MODEL_POINTS+D, 1, points2.type());
			} else {
				_output1.create(MIN_MODEL_POINTS+D, 2, points1.type());
				_output2.create(MIN_MODEL_POINTS+D, 2, points2.type());
			}

			Mat output1 = _output1.getMat();
			Mat output2 = _output2.getMat();

			if(selection.size() != count){
				selection = vector<int>();
				for (int i=0; i<count; i++) selection.push_back(i);
			}

			if(useSimilarityScore){
				currIter++;

				if (currIter >= stageIters && stageN < count) {
					stageN++;
					double Tn1 = Tn * (double)(stageN+1) / (stageN+1-MIN_MODEL_POINTS);
					double stageIts = Tn1 - Tn;
					stageIters = (int)ceil(stageIts);
					currIter = 0;
					Tn = Tn1;
				}

				random_shuffle(selection.begin(), selection.begin()+stageN);
			} else {
				random_shuffle(selection.begin(), selection.end());
			}
			for(int j=0; j<MIN_MODEL_POINTS+D; j++){
				points1.row(selection[j]).copyTo(output1.row(j));
				points2.row(selection[j]).copyTo(output2.row(j));
			}

		}
};

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
		case SFM_MLESAC :
			return (new MLESAC_Estimator(param1, param2, param3));
			break;
		case SFM_ARRSAC :
			return (new ARRSAC_Estimator(param1, param2, param3));
			break;
		default :
			return NULL;
	}
}