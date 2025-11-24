// WienerPost.cpp
#include "WienerPost.hpp"
#include <algorithm>
#include <cmath>

void WienerPost::applyRealPacked(float* X, int N)
{
    const int K = N/2;
    if ((int)Pn_.size() != K+1) { Pn_.assign(K+1, 1e-6f); Sprior_.assign(K+1, 0.0f); }

    const float alpha = 0.98f;
    const float noiseAlpha = 0.95f; // Slower adaptation for stability

    // FIX: Make the maximum reduction depend on the Strength Knob.
    // If strength is 0, max reduction is 0dB.
    // If strength is 100%, max reduction is -40dB (previously stuck at -12dB).
    float maxReductionDb = -40.0f * strength_; 
    const float Gmin = std::pow(10.0f, maxReductionDb / 20.0f);

    auto mag2 = [&](int k)->float {
        if (k==0)  return X[0]*X[0];
        if (k==K)  return X[1]*X[1];
        const int re = 2*k, im = 2*k+1;
        if (im >= 2*N) return 0.0f;
        float r=X[re], ii=X[im]; return r*r + ii*ii;
    };

    for (int k=0;k<=K;++k){
        float Y2 = mag2(k) + 1e-12f;
        
        // Estimate noise floor (Minimum Statistics)
        Pn_[k] = noiseAlpha*Pn_[k] + (1.0f-noiseAlpha)*std::min(Pn_[k], Y2);

        float postSNR  = std::clamp(Y2/(Pn_[k] + 1e-12f), 0.0f, 1000.0f);
        float priorSNR = alpha*(Sprior_[k]/(Pn_[k]+1e-12f))
                        + (1.0f-alpha)*std::max(postSNR - 1.0f, 0.0f);

        float G = priorSNR / (1.0f + priorSNR);
        
        // Limit the Gain reduction so it doesn't go below Gmin
        if (G < Gmin) G = Gmin;

        if (k==0)      X[0] *= G;
        else if (k==K) X[1] *= G;
        else           { X[2*k] *= G; X[2*k+1] *= G; }

        Sprior_[k] = G*G * Y2;
    }
}