//
// Created by SBNut on 2025-11-04.
//

#include "NLMSFilter.hpp"
#include <algorithm>
#include <cmath>


void NLMSFilter::prepare(double sr, int L){ sr_ = sr; setLength(L); }


void NLMSFilter::setLength(int L){
L_ = std::max(8, L);
w_.assign(L_, 0.0f);
xbuf_.assign(L_, 0.0f);
idx_ = 0;
}


float NLMSFilter::processOne(float primary, float reference){
// write new reference sample
xbuf_[idx_] = reference;


// estimate noise via FIR dot product
double yhat = 0.0;
double norm = eps_;
size_t p = idx_;
for (int i=0;i<L_;++i){
float xi = xbuf_[p];
yhat += w_[i] * xi;
norm += double(xi) * double(xi);
p = (p==0) ? size_t(L_-1) : p-1;
}


float e = primary - (float) yhat; // canceller output


if (!frozen_){
const double g = mu_ * e / norm; // normalized step
p = idx_;
for (int i=0;i<L_;++i){
w_[i] += (float)(g * xbuf_[p]);
p = (p==0) ? size_t(L_-1) : p-1;
}
}


// advance ring index
idx_ = (idx_ + 1) % (size_t)L_;
return e;
}
