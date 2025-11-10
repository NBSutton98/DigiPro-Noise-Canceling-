//
// Created by SBNut on 2025-11-04.
//

#include "WienerPost.h"
#include <juce_core/juce_core.h>
#include <cmath>


void WienerPost::applyRealPacked(float* X, int N){
const int K = N/2; // usable bins [0..K]
if (!init_) { Pn_.assign(K+1, 1e-6f); Sprior_.assign(K+1, 0.0f); init_ = true; }


const float alpha = 0.98f; // decision-directed smoothing
const float noiseAlpha = 0.95f; // noise PSD smoothing
const float Gmin = juce::Decibels::decibelsToGain(-12.0f); // floor


// magnitude^2 per bin from JUCE packed format
auto mag2 = [&](int k){ if (k==0) return X[0]*X[0]; if (k==K) return X[1]*X[1];
float re=X[2*k], im=X[2*k+1]; return re*re+im*im; };


for (int k=0;k<=K;++k){
float Y2 = mag2(k) + 1e-12f;


// Update noise PSD via minima tracking proxy (here: slow EMA on low-energy assumption)
Pn_[k] = noiseAlpha * Pn_[k] + (1.0f - noiseAlpha) * std::min(Pn_[k], Y2);


// Decision-directed prior SNR
float postSNR = juce::jlimit(0.0f, 1000.0f, Y2 / (Pn_[k] + 1e-12f));
float priorSNR = alpha * (Sprior_[k] / (Pn_[k] + 1e-12f)) + (1.0f - alpha) * std::max(postSNR - 1.0f, 0.0f);


// Wiener gain
float G = priorSNR / (1.0f + priorSNR);
G = 1.0f - strength_ * (1.0f - G); // user strength toward 0
if (G < Gmin) G = Gmin;


// Apply gain to packed spectrum
if (k==0) { X[0] *= G; }
else if (k==K) { X[1] *= G; }
else { X[2*k] *= G; X[2*k+1] *= G; }


// Update prior power estimate for next frame
Sprior_[k] = G*G * Y2;
}
}
