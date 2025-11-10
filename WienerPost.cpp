#include "WienerPost.hpp"
#include <cmath>

// WienerPost.cpp
void WienerPost::applyRealPacked(float* X, int N){
    const int K = N/2;
    if ((int)Pn_.size() != K+1) {            // <-- resize every call
        Pn_.assign(K+1, 1e-6f);
        Sprior_.assign(K+1, 0.0f);
    }

    const float alpha = 0.98f;
    const float noiseAlpha = 0.95f;
    const float Gmin = std::pow(10.0f, -12.0f/20.0f);  // -12 dB

    auto mag2 = [&](int k)->float {
        if (k==0)  return X[0]*X[0];
        if (k==K)  return X[1]*X[1];
        const int re = 2*k, im = 2*k+1;
        // hard guard against malformed buffers
        if (im >= 2*N) return 0.0f;
        float r=X[re], ii=X[im]; return r*r + ii*ii;
    };

    for (int k=0;k<=K;++k){
        float Y2 = mag2(k) + 1e-12f;
        Pn_[k] = noiseAlpha*Pn_[k] + (1.0f-noiseAlpha)*std::min(Pn_[k], Y2);
        float postSNR  = juce::jlimit(0.0f, 1000.0f, Y2/(Pn_[k] + 1e-12f));
        float priorSNR = alpha*(Sprior_[k]/(Pn_[k]+1e-12f))
                       + (1.0f-alpha)*std::max(postSNR - 1.0f, 0.0f);

        float G = priorSNR / (1.0f + priorSNR);
        G = 1.0f - strength_ * (1.0f - G);
        if (G < Gmin) G = Gmin;

        if (k==0)      X[0] *= G;
        else if (k==K) X[1] *= G;
        else           { X[2*k] *= G; X[2*k+1] *= G; }

        Sprior_[k] = G*G * Y2;
    }
}

