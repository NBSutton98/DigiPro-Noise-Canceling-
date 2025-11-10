//
// Created by SBNut on 2025-11-04.
//

#ifndef NLMSFILTER_HPP
#define NLMSFILTER_HPP

#pragma once
#include <vector>


class NLMSFilter {
public:
void prepare(double sr, int L);
void setLength(int L);
void setMu(double mu) { mu_ = mu; }
void setFrozen(bool f) { frozen_ = f; }


float processOne(float primary, float reference);


private:
double sr_{48000.0};
int L_{256};
double mu_{0.2};
bool frozen_{false};
double eps_{1e-8};


std::vector<float> w_;
std::vector<float> xbuf_;
size_t idx_{0};
};



#endif //NLMSFILTER_HPP
